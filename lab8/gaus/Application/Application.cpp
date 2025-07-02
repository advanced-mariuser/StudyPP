#include "Application.h"
#include "../BlurProcessor/GaussianBlurProcessor.h"
#include "../BlurProcessor/MotionBlurProcessor.h"
#include <algorithm>
#include <iostream>

Application::Application()
	: m_currentProcessor(nullptr)
	, m_activeFilterType(ActiveFilterType::GAUSSIAN_BLUR)
{
}

bool Application::Initialize(int argc, char** argv)
{
	std::string imagePathArg = "me.jpg";
	if (argc > 1)
	{
		imagePathArg = argv[1];
	}
	else
	{
		std::cout << "No image path provided, using default: " << imagePathArg << std::endl;
	}
	m_imagePath = imagePathArg;

	const cv::Mat originalInputImage = cv::imread(m_imagePath, cv::IMREAD_COLOR);
	if (originalInputImage.empty())
	{
		std::cerr << "Application Error: Could not open or find the image: " << m_imagePath << std::endl;
		return false;
	}

	if (originalInputImage.channels() == 3)
	{
		cv::cvtColor(originalInputImage, m_inputImageBGRA, cv::COLOR_BGR2BGRA);
	}
	else if (originalInputImage.channels() == 4)
	{
		m_inputImageBGRA = originalInputImage.clone();
	}
	else if (originalInputImage.channels() == 1)
	{
		cv::cvtColor(originalInputImage, m_inputImageBGRA, cv::COLOR_GRAY2BGRA);
	}
	else
	{
		std::cerr << "Application Error: Unsupported number of image channels: " << originalInputImage.channels() << std::endl;
		return false;
	}

	if (!m_oclManager.Initialize())
	{
		std::cerr << "Application Error: Failed to initialize OpenCLManager." << std::endl;
		return false;
	}

	if (!m_oclManager.LoadProgramFromFile(m_kernelFilePath))
	{
		std::cerr << "Application Error: Failed to load OpenCL program from " << m_kernelFilePath << std::endl;
		return false;
	}

	auto gaussianBlur = std::make_unique<GaussianBlurProcessor>();
	if (gaussianBlur->Initialize(m_oclManager))
	{
		m_allProcessors.push_back(std::move(gaussianBlur));
	}
	else
	{
		std::cerr << "Failed to initialize GaussianBlurProcessor" << std::endl;
	}

	auto motionBlur = std::make_unique<MotionBlurProcessor>();
	if (motionBlur->Initialize(m_oclManager))
	{
		m_allProcessors.push_back(std::move(motionBlur));
	}
	else
	{
		std::cerr << "Failed to initialize MotionBlurProcessor" << std::endl;
	}

	if (m_allProcessors.empty())
	{
		std::cerr << "Application Error: No image processors were successfully initialized." << std::endl;
		return false;
	}

	m_outputImage.create(m_inputImageBGRA.size(), m_inputImageBGRA.type());

	const int MAX_WINDOW_WIDTH = 1920;
	const int MAX_WINDOW_HEIGHT = 1080;
	cv::namedWindow(m_windowName, cv::WINDOW_NORMAL);
	int windowW = m_inputImageBGRA.cols;
	int windowH = m_inputImageBGRA.rows;

	if (windowW > MAX_WINDOW_WIDTH)
	{
		float ratio = static_cast<float>(MAX_WINDOW_WIDTH) / windowW;
		windowW = MAX_WINDOW_WIDTH;
		windowH = static_cast<int>(windowH * ratio);
	}
	if (windowH > MAX_WINDOW_HEIGHT)
	{
		float ratio = static_cast<float>(MAX_WINDOW_HEIGHT) / windowH;
		windowH = MAX_WINDOW_HEIGHT;
		windowW = static_cast<int>(windowW * ratio);
	}
	windowW = std::max(1, windowW);
	windowH = std::max(1, windowH);
	cv::resizeWindow(m_windowName, windowW, windowH);

	for (const auto& proc : m_allProcessors)
	{
		if (proc)
		{
			proc->CreateTrackbars(m_windowName);
		}
	}

	SwitchFilter(m_activeFilterType);

	if (m_currentProcessor)
	{
		m_currentProcessor->OnParametersChanged(m_windowName);
		if (!m_currentProcessor->ProcessImage(m_inputImageBGRA, m_outputImage))
		{
			std::cerr << "Application Error: Initial image processing failed for "
					  << m_currentProcessor->GetName() << std::endl;
		}
		if (!m_outputImage.empty())
		{
			cv::imshow(m_windowName, m_outputImage);
		}
		else if (!m_inputImageBGRA.empty())
		{
			cv::imshow(m_windowName, m_inputImageBGRA);
		}
	}
	else
	{
		std::cerr << "Application Error: No current processor set after initialization." << std::endl;
		if (!m_inputImageBGRA.empty())
		{
			cv::imshow(m_windowName, m_inputImageBGRA);
		}
	}
	return true;
}

void Application::SwitchFilter(ActiveFilterType filterType)
{
	ImageProcessor* targetProcessor = nullptr;
	std::string targetName;

	switch (filterType)
	{
	case ActiveFilterType::GAUSSIAN_BLUR:
		targetName = "Gaussian Blur";
		break;
	case ActiveFilterType::MOTION_BLUR:
		targetName = "Motion Blur";
		break;
	case ActiveFilterType::NONE:
		m_currentProcessor = nullptr;
		m_activeFilterType = ActiveFilterType::NONE;
		std::cout << "No filter selected." << std::endl;
		if (!m_inputImageBGRA.empty())
		{
			cv::imshow(m_windowName, m_inputImageBGRA);
		}
		return;
	default:
		std::cerr << "Application Warning: Unknown filter type requested for switching." << std::endl;
		return;
	}

	for (const auto& proc : m_allProcessors)
	{
		if (proc->GetName() == targetName)
		{
			targetProcessor = proc.get();
			break;
		}
	}

	if (targetProcessor)
	{
		if (m_currentProcessor == targetProcessor && m_activeFilterType == filterType)
		{
			std::cout << "Filter " << targetProcessor->GetName() << " already active. Updating parameters." << std::endl;
			m_currentProcessor->OnParametersChanged(m_windowName);
			return;
		}

		m_currentProcessor = targetProcessor;
		m_activeFilterType = filterType;
		std::cout << "Switched to filter: " << m_currentProcessor->GetName() << std::endl;

		m_currentProcessor->OnParametersChanged(m_windowName);
	}
	else
	{
		std::cerr << "Application Error: Could not find processor for type " << targetName << std::endl;
		m_currentProcessor = nullptr;
		m_activeFilterType = ActiveFilterType::NONE;
	}
}

void Application::MainLoop()
{
	std::cout << "Application started. Press 'Esc' to exit. Press '1' for Gaussian, '2' for Motion Blur." << std::endl;

	while (true)
	{
		int key = cv::waitKey(30);

		if (key == 27)
		{
			break;
		}
		if (key == 'g')
		{
			SwitchFilter(ActiveFilterType::GAUSSIAN_BLUR);
		}
		else if (key == 'm')
		{
			SwitchFilter(ActiveFilterType::MOTION_BLUR);
		}

		if (m_currentProcessor)
		{
			m_currentProcessor->OnParametersChanged(m_windowName);

			if (!m_currentProcessor->ProcessImage(m_inputImageBGRA, m_outputImage))
			{
				std::cerr << "Application Error: Image processing failed in loop for "
						  << m_currentProcessor->GetName() << std::endl;
				if (!m_inputImageBGRA.empty())
					cv::imshow(m_windowName, m_inputImageBGRA);
			}
			else
			{
				if (!m_outputImage.empty())
				{
					cv::imshow(m_windowName, m_outputImage);
				}
			}
		}
		else
		{
			if (!m_inputImageBGRA.empty())
			{
				cv::imshow(m_windowName, m_inputImageBGRA);
			}
		}
	}
}

void Application::Cleanup()
{
	cv::destroyAllWindows();
}

int Application::Run(int argc, char** argv)
{
	if (!Initialize(argc, argv))
	{
		std::cerr << "Application initialization failed." << std::endl;
		return EXIT_FAILURE;
	}
	MainLoop();
	Cleanup();
	std::cout << "Application finished." << std::endl;
	return EXIT_SUCCESS;
}
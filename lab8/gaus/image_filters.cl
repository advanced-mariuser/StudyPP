//0.04045, 0.0031308, 12.92, 1.055 — это стандартизированные константы sRGB

inline float srgb_to_linear_channel(float c)
{
    if (c <= 0.0f)
    {
        return 0.0f;
    }

    if (c <= 0.04045f)
    {
        return c / 12.92f;
    }
    else
    {
        return pow((c + 0.055f) / 1.055f, 2.4f);
    }
}

inline float linear_to_srgb_channel(float c)
{
    if (c <= 0.0f)
    {
        return 0.0f;
    }
    if (c <= 0.0031308f)
    {
        return c * 12.92f;
    }
    else
    {
        return 1.055f * pow(c, 1.0f / 2.4f) - 0.055f;
    }
}

inline float4 bgra_uchar_to_linear_float(uchar4 pixel)
{
    float4 floatPixel = convert_float4(pixel) / 255.0f;
    return (float4)(
        srgb_to_linear_channel(floatPixel.x), // B
        srgb_to_linear_channel(floatPixel.y), // G
        srgb_to_linear_channel(floatPixel.z), // R
        floatPixel.w                          // A
    );
}

inline uchar4 linear_float_to_bgra_uchar(float4 linearPixel)
{
    float4 srgbPixel = (float4)(
        linear_to_srgb_channel(linearPixel.x), // B
        linear_to_srgb_channel(linearPixel.y), // G
        linear_to_srgb_channel(linearPixel.z), // R
        linearPixel.w                          // A
    );
    return convert_uchar4_sat(srgbPixel * 255.0f);
}

__kernel void transpose_rgba(__global const uchar4* input, __global uchar4* output, const int width, const int height)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    if (x >= width || y >= height) return;

    uchar4 pixel = input[y * width + x];

    output[x * height + y] = pixel;
}

__kernel void gaussian_blur_horizontal_rgba(
    __global const uchar4* inputImage,
    __global uchar4* outputImage,
    __constant float* kernelWeights,
    int imageWidth,
    int imageHeight,
    int kernelRadius)
{
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);

    if (gidX >= imageWidth || gidY >= imageHeight)
    {
        return;
    }

    if (kernelRadius == 0)
    {
        outputImage[gidY * imageWidth + gidX] = inputImage[gidY * imageWidth + gidX];
        return;
    }

    float4 sumValueLinear = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float sumAlpha = 0.0f;

    // Проход по всем пикселям в пределах радиуса размытия
    for (int kOffset = -kernelRadius; kOffset <= kernelRadius; ++kOffset)
    {
        // Вычисляем координату соседнего пикселя с обработкой границ
        int currentX = clamp(gidX + kOffset, 0, imageWidth - 1);
        uchar4 pixelUchar = inputImage[gidY * imageWidth + currentX];
        // Конвертируем цвет из sRGB в линейное пространство
        float4 pixelLinear = bgra_uchar_to_linear_float(pixelUchar);

        // Получаем вес из гауссова ядра
        float weightValue = kernelWeights[kOffset + kernelRadius];

        // Накопление взвешенной суммы цветов
        sumValueLinear.xyz += pixelLinear.xyz * weightValue;
        // Отдельное накопление альфа-канала
        sumAlpha += pixelLinear.w * weightValue;
    }
    sumValueLinear.w = sumAlpha;

    // Конвертируем результат обратно в sRGB и записываем в выходное изображение
    outputImage[gidY * imageWidth + gidX] = linear_float_to_bgra_uchar(sumValueLinear);
}

//TODO: image_t
__kernel void motion_blur_horizontal_rgba(
    __global const uchar4* inputImage,
    __global uchar4* outputImage,
    int imageWidth,
    int imageHeight,
    int blurLength)
{
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);

    if (gidX >= imageWidth || gidY >= imageHeight)
    {
        return;
    }

    if (blurLength <= 1)
    {
        outputImage[gidY * imageWidth + gidX] = inputImage[gidY * imageWidth + gidX];
        return;
    }

    float4 sumValue = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    // Радиус ядра размытия
    int kernelRadius = (blurLength - 1) / 2;
    int actualKernelSize = 0;

    // Проходим по всем пикселям в пределах радиуса размытия
    for (int kOffset = -kernelRadius; kOffset <= kernelRadius; ++kOffset)
    {
        int currentXOffset = gidX + kOffset;
        if (currentXOffset >= 0 && currentXOffset < imageWidth)
        {
            // Получаем значение соседнего пикселя
            uchar4 pixelValue = inputImage[gidY * imageWidth + currentXOffset];
            sumValue += convert_float4(pixelValue);
            actualKernelSize++;
        }
    }

    if (actualKernelSize > 0)
    {
        //Делим сумму на количество учтенных пикселей (среднее арифметическое)
        sumValue /= (float)actualKernelSize;
    }
    else
    {
        sumValue = convert_float4(inputImage[gidY * imageWidth + gidX]);
    }

    outputImage[gidY * imageWidth + gidX] = convert_uchar4_sat(sumValue);
}
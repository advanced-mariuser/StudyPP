#include <iostream>
#include <vector>
#include <coroutine>

struct Book
{
    std::string title;
    std::string author;
    std::vector<std::string> chapters;
};

struct BookChapter
{
    std::string bookTitle;
    std::string bookAuthor;
    std::string chapterTitle;
};

std::ostream& operator<<(std::ostream& os, const BookChapter& chapter)
{
    return os << chapter.bookTitle << " by " << chapter.bookAuthor
              << " - " << chapter.chapterTitle;
}
//TODO: у value должен быть конструктор по умолчанию
//TODO: мы храним в генераторе слишком долго значение
//TODO: возможно сделать 2 версии yield_value
template<typename T>
struct Generator
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        T value;
        std::exception_ptr exception;

        Generator get_return_object()
        {
            return Generator(handle_type::from_promise(*this));
        }

        std::suspend_always initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; }

        void unhandled_exception() { exception = std::current_exception(); }

        std::suspend_always yield_value(T val)
        {
            value = std::move(val);
            return {};
        }

        void return_void() {}
    };

    explicit Generator(handle_type h) : m_handle(h) {}

    ~Generator() { if (m_handle) m_handle.destroy(); }

    struct iterator
    {
        handle_type coro;

        bool operator!=(std::default_sentinel_t) const { return !coro.done(); }

        void operator++() { coro.resume(); }

        const T& operator*() const { return coro.promise().value; }
    };

    iterator begin()
    {
        if (m_handle)
        {
            m_handle.resume();
        }
        return iterator{m_handle};
    }

    std::default_sentinel_t end() { return {}; }

private:
    handle_type m_handle;
};

Generator<BookChapter> ListBookChapters(const std::vector<Book>& books)
{
    for (const auto& book: books)
    {
        for (const auto& chapterTitle: book.chapters)
        {
            co_yield BookChapter{book.title, book.author, chapterTitle};
        }
    }
}

int main()
{
    std::vector<Book> books = {
            {"The Great Gatsby",      "F. Scott Fitzgerald", {"Chapter 1", "Chapter 2"}},
            {"1984",                  "George Orwell",       {"Chapter 1", "Chapter 2", "Chapter 3"}},
            {"To Kill a Mockingbird", "Harper Lee",          {"Chapter 1"}}
    };

    for (const auto& chapter: ListBookChapters(books))
    {
        std::cout << chapter << std::endl;
    }
}
#pragma once

#include <future>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <cassert>

namespace SonaHttp 
{

class ThreadPool 
{
private:
    template<typename _type>
    using result_of_t = typename std::result_of<_type>::type;

    template<typename _fCallable, typename... _tParams>
    using packed_task_t
            = std::packaged_task<result_of_t<_fCallable&&(_tParams&&...)>()>;

    template<typename _fCallable, typename... _tParams>
    std::shared_ptr<packed_task_t<_fCallable&&, _tParams&&...>>
    pack_shared_task(_fCallable&& f, _tParams&&... args)
    {
            return std::make_shared<packed_task_t<_fCallable&&, _tParams&&...>>(
            std::bind(std::forward<_fCallable>(f), std::forward<_tParams>(args)...));
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks{};
    mutable std::mutex queue_mutex{};
    std::condition_variable condition{};

    bool stopped = {};

public:
 
    ThreadPool(size_t, std::function<void()> = {}, std::function<void()> = {});

    ~ThreadPool() noexcept;

    template<typename _fCallable, typename... _tParams>
    std::future<result_of_t<_fCallable&&(_tParams&&...)>>
    enqueue(_fCallable&& f, _tParams&&... args)
    {
        return wait_to_enqueue([](std::unique_lock<std::mutex>&){}, 
            std::forward<_fCallable>(f), std::forward<_tParams>(args)...);
    }

    size_t size() const;  
    size_t size_unlocked() const noexcept
    {
        return tasks.size();
    }

    template<typename _fWaiter, typename _fCallable, typename... _tParams>
    std::future<result_of_t<_fCallable&&(_tParams&&...)>>
    wait_to_enqueue(_fWaiter wait, _fCallable&& f, _tParams&&... args)
    {
        const auto
            task(pack_shared_task(
            std::forward<_fCallable>(f), std::forward<_tParams>(args)...));
        auto res(task->get_future());

        {
            std::unique_lock<std::mutex> lck(queue_mutex);

            wait(lck);
            assert(lck.owns_lock());
            tasks.push([task]{
                (*task)();
            });
        }
        condition.notify_one();
        return std::move(res);
    }
};

}    /**< namespace SonaHttp */

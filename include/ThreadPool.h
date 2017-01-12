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

/*  * Brief Thread pool.
  * Note Unless otherwise agreed, all public member functions are thread-safe.
  * Note The length of the thread queue is not controlled.
  * */
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
    /*
      * Brief Stop status.
      * Note Set stop only when destructor.
     */
    bool stopped = {};

public:
  /*
      * Brief Initialization: Use the specified initialization and exit callback to specify the number of worker threads.
      * Note If the callback is empty is ignored.
      * Warning Callback execution does not provide sequential and concurrent security guarantees.
   */
    ThreadPool(size_t, std::function<void()> = {}, std::function<void()> = {});

    /*
      * Brief Destructor: set the stop state and wait for all the execution of the end of the thread.
      * Note Assertions Set stop state without throwing \ c std :: system_error.
      * Note Possible blocking. Ignore each stalled \ c std :: system_error.
      * Note No exception Thrown: assertion.
      */
    ~ThreadPool() noexcept;

    /** \see wait_to_enqueue. */
    template<typename _fCallable, typename... _tParams>
    std::future<result_of_t<_fCallable&&(_tParams&&...)>>
    enqueue(_fCallable&& f, _tParams&&... args)
    {
        return wait_to_enqueue([](std::unique_lock<std::mutex>&){}, 
            std::forward<_fCallable>(f), std::forward<_tParams>(args)...);
    }

    size_t size() const;

   //Warning Non-thread safe.
      
    size_t
    size_unlocked() const noexcept
    {
        return tasks.size();
    }

  /**
      * Brief Wait for the operation to enter the queue.
      * Param f Prepare to enter the queue operation
      * Param args Parameters to enter the queue operation.
      * Param wait Wait for operation.
      * Pre wait After the call to meet the condition variable postcondition; Assertion: Hold lock.
      * Warning needs to ensure that the task has not been stopped (not entered into the destructor), otherwise the task is not guaranteed to be run.
      * Warning Use non-recursive lock, can not be locked again when waiting.
      */
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

namespace olc
{
    namespace net
    {
        template<typename T>
        class tsqueue
        {
            public:
                tsqueue() = default;
                tsqueue(const tsqueue<T>&) = delete;
                // destructor
                virtual ~tsqueue() {clear();}
                
            public:
                // Returns and maintains front of queue
                const T& front()
                {
                    std::scoped_lock lock(muxQueue);
                    return deqQueue.front();
                }

                // Returns and maintains item at the back of queue
                const T& back()
                {
                    std::scoped_lock lock(muxQueue);
                    return deQueue.back();
                }

                // Adds an item to the back of queue
                void push_back(const T& item)
                {
                    std::scoped_lock lock(muxQueue);
                    deQueue.emplace_back(std::move(item));
                }

                // Adds an item to the front of queue
                void push_front(const T& item)
                {
                    std::scoped_lock lock(muxQueue);
                    deqQueue.emplace_front(std::move(item));
                }

                // Returns true if Queue has no items
                bool empty()
                {
                    std::scoped_lock lock(muxQueue);
                    return deqQueue.empty();
                }

                // Returns number of items in Queue
                size_t count()
                {
                    std::scoped_lock lock(muxQueue);
                    return deqQueue.size();
                }

                // Clears Queue
                void clear()
                {
                    std::scoped_lock lock(muxQueue);
                    deqQueue.clear();
                }

                // Removes and  returns item from the front of the Queue
                T pop_front()
                {
                    std::scoped_lock lock(muxQueue);
                    auto t = std::move(deqQueue.front());
                    deqQueue.pop_front();
                    return t;
                }

                T pop_back()
                {
                    std::scoped_lock lock(muxQueue);
                    auto t = std::move(deqQueue.back());
                    deqQueue.pop_back();
                    return t;
                }

            protected:
                std::mutex muxQueue;
                std::deque<T> deqQueue;
        };
    }
}
#ifndef MEXEDIT_MEMORY_DEBUGGER_H
#define MEXEDIT_MEMORY_DEBUGGER_H

#include <memory>
#include <unordered_map>
#include <string>
#include <iostream>
#include <mutex>

/// @brief Utilities namespace for mexEdit \namespace mexedit::utils
namespace mexedit::utils
{
    /// @brief Simple memory usage tracker for debugging purposes \class MemoryDebugger
    class MemoryDebugger
    {
    public:
        /**
         * @brief Get the Instance object
         * 
         * @return MemoryDebugger& 
         */
        static MemoryDebugger& getInstance()
        {
            static MemoryDebugger instance;
            return instance;
        }
        
        /**
         * @brief Track object creation
         * 
         * @tparam T The object type
         * @param objectType The type of the object as a string
         * @param ptr A pointer to the created object
         */
        template<typename T>
        void trackCreation(const std::string& objectType, const T* ptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            objectCounts_[objectType]++;
            totalObjects_++;
            (void)ptr;
            #ifdef DEBUG
            std::cout << "[DEBUG] Created " << objectType << " (Total: " << objectCounts_[objectType] << ")" << std::endl;
            #endif
        }
        
        /**
         * @brief Track object destruction
         * 
         * @tparam T The object type
         * @param objectType The type of the object as a string
         * @param ptr A pointer to the destroyed object
         */
        template<typename T>
        void trackDestruction(const std::string& objectType, const T* ptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (objectCounts_[objectType] > 0)
            {
                objectCounts_[objectType]--;
                totalObjects_--;
                (void)ptr;
            }
            
            #ifdef DEBUG
            std::cout << "[DEBUG] Destroyed " << objectType << " (Remaining: " << objectCounts_[objectType] << ")" << std::endl;
            #endif
        }
        
        /**
         * @brief Print current memory statistics
         * 
         */
        void printStats() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << "\n=== Memory Debug Statistics ===" << std::endl;
            std::cout << "Total objects alive: " << totalObjects_ << std::endl;
            
            for (const auto& [type, count] : objectCounts_)
            {
                if (count > 0)
                {
                    std::cout << "  " << type << ": " << count << std::endl;
                }
            }
            std::cout << "==============================\n" << std::endl;
        }
        
        /**
         * @brief Check if there are potential memory leaks
         * 
         * @return true if there are potential leaks, false otherwise
         */
        bool hasLeaks() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return totalObjects_ > 0;
        }

    private:
        /**
         * @brief Construct a new Memory Debugger object
         * 
         */
        MemoryDebugger() = default;

        /**
         * @brief Destroy the Memory Debugger object
         * 
         */
        ~MemoryDebugger() = default;

        /**
         * @brief Copy constructor (deleted)
         * 
         */
        MemoryDebugger(const MemoryDebugger&) = delete;

        /**
         * @brief Copy assignment operator (deleted)
         * 
         */
        MemoryDebugger& operator=(const MemoryDebugger&) = delete;
        
        mutable std::mutex mutex_;
        std::unordered_map<std::string, size_t> objectCounts_;
        size_t totalObjects_ = 0;
    };
    
    /**
     * @brief RAII helper class for automatic tracking
     * 
     * @tparam T The object type
     */
    template<typename T>
    class MemoryTracker
    {
    public:
        /**
         * @brief Construct a new Memory Tracker object
         * 
         * @param objectType The type of the object as a string
         * @param ptr A pointer to the tracked object
         */
        MemoryTracker(const std::string& objectType, const T* ptr)
            : objectType_(objectType)
            , ptr_(ptr)
        {
            MemoryDebugger::getInstance().trackCreation(objectType_, ptr_);
        }
        
        /**
         * @brief Destroy the Memory Tracker object
         * 
         */
        ~MemoryTracker()
        {
            MemoryDebugger::getInstance().trackDestruction(objectType_, ptr_);
        }
        
    private:
        std::string objectType_;
        const T* ptr_;
    };
}

#ifdef DEBUG
    #define TRACK_MEMORY(type, ptr) \
        mexedit::utils::MemoryTracker<std::remove_pointer_t<decltype(ptr)>> \
        tracker_##__LINE__(#type, ptr)
#else
    #define TRACK_MEMORY(type, ptr) do {} while(0)
#endif

#endif // MEXEDIT_MEMORY_DEBUGGER_H

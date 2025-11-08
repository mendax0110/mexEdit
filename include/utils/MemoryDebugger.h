#ifndef MEXEDIT_MEMORY_DEBUGGER_H
#define MEXEDIT_MEMORY_DEBUGGER_H

#include <memory>
#include <unordered_map>
#include <string>
#include <iostream>
#include <mutex>
#include <utility>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <set>

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

            const void* voidPtr = static_cast<const void*>(ptr);
            if (allTrackedPointers_.count(voidPtr))
            {
                return;
            }

            objectCounts_[objectType]++;
            totalObjects_++;
            allTrackedPointers_.insert(voidPtr);
            objectPointers_[objectType].insert(voidPtr);
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

            const void* voidPtr = static_cast<const void*>(ptr);

            if (!allTrackedPointers_.count(voidPtr))
            {
                return;
            }

            if (objectCounts_[objectType] > 0)
            {
                objectCounts_[objectType]--;
                totalObjects_--;
                allTrackedPointers_.erase(voidPtr);
                objectPointers_[objectType].erase(voidPtr);
            }
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

        /**
         * @brief Generate comprehensive memory map of the application
         *
         */
        void describeLeakAndMemoryMap()
        {
            std::lock_guard<std::mutex> lock(mutex_);

            std::cout << "\n";
            std::cout << "================================================================================\n";
            std::cout << "                          APPLICATION MEMORY MAP\n";
            std::cout << "================================================================================\n";
            std::cout << "Total Objects Allocated: " << totalObjects_ << "\n";
            std::cout << "Object Types Tracked:    " << objectCounts_.size() << "\n";
            std::cout << "--------------------------------------------------------------------------------\n";

            if (totalObjects_ == 0)
            {
                std::cout << "STATUS: No memory leaks detected - all objects properly deallocated\n";
                std::cout << "================================================================================\n\n";
                return;
            }

            size_t maxTypeWidth = 20;
            size_t maxCountWidth = 8;

            for (const auto& [type, count] : objectCounts_)
            {
                maxTypeWidth = std::max(maxTypeWidth, type.length());
                maxCountWidth = std::max(maxCountWidth, std::to_string(count).length());
            }

            std::cout << std::left << std::setw(maxTypeWidth) << "OBJECT TYPE"
                      << " | " << std::setw(maxCountWidth) << "COUNT"
                      << " | STATUS\n";
            std::cout << std::string(maxTypeWidth, '-') << "-|-"
                      << std::string(maxCountWidth, '-') << "-|--------\n";

            std::vector<std::pair<std::string, size_t>> sortedObjects(objectCounts_.begin(), objectCounts_.end());

            std::sort(sortedObjects.begin(), sortedObjects.end(), [](const auto& a, const auto& b)
            {
                return a.second > b.second;
            });

            for (const auto& [type, count] : sortedObjects)
            {
                std::string status = (count > 0) ? "LEAKED" : "CLEAN";
                std::cout << std::left << std::setw(maxTypeWidth) << type
                          << " | " << std::right << std::setw(maxCountWidth) << count
                          << " | " << status << "\n";
            }

            std::cout << "--------------------------------------------------------------------------------\n";

            if (totalObjects_ > 0)
            {
                std::cout << "MEMORY LEAK ANALYSIS:\n";
                std::cout << "  - Total leaked objects: " << totalObjects_ << "\n";

                size_t leakedTypes = 0;
                size_t maxLeakCount = 0;
                std::string worstLeaker;

                for (const auto& [type, count] : objectCounts_)
                {
                    if (count > 0)
                    {
                        leakedTypes++;
                        if (count > maxLeakCount)
                        {
                            maxLeakCount = count;
                            worstLeaker = type;
                        }
                    }
                }

                std::cout << "  - Object types with leaks: " << leakedTypes << "\n";
                std::cout << "  - Primary leak source: " << worstLeaker
                          << " (" << maxLeakCount << " instances)\n";

                std::cout << "\nMEMORY DISTRIBUTION:\n";
                for (const auto& [type, count] : sortedObjects)
                {
                    if (count > 0)
                    {
                        double percentage = (static_cast<double>(count) / totalObjects_) * 100.0;
                        std::cout << "  - " << std::setw(maxTypeWidth) << std::left << type
                                  << ": " << std::setw(5) << std::right << std::fixed
                                  << std::setprecision(1) << percentage << "% (" << count << " objects)\n";
                    }
                }

                std::cout << "\nLEAKED OBJECT ADDRESSES:\n";
                for (const auto& [type, pointers] : objectPointers_)
                {
                    if (!pointers.empty())
                    {
                        std::cout << "  " << type << ":\n";
                        for (const void* ptr : pointers)
                        {
                            std::cout << "    - " << ptr << "\n";
                        }
                    }
                }
            }

            std::cout << "================================================================================\n\n";

            printDebugSuggestions();
        }

        /**
         * @brief Print debugging suggestions based on leak patterns
         */
        void printDebugSuggestions() const
        {
            std::cout << "DEBUGGING SUGGESTIONS:\n";
            std::cout << "======================\n";

            // Check for double initialization pattern
            bool hasDoubles = false;
            for (const auto& [type, count] : objectCounts_)
            {
                if (count == 2)
                {
                    std::cout << "- " << type << " has exactly 2 instances - check for copy instead of move\n";
                    hasDoubles = true;
                }
            }

            if (hasDoubles)
            {
                std::cout << "\nPOTENTIAL ISSUES IDENTIFIED:\n";
                std::cout << "1. Objects being copied instead of moved in constructors\n";
                std::cout << "2. Missing move constructors/assignment operators\n";
                std::cout << "3. Check TRACK_MEMORY placement in copy/move constructors\n";
            }

            if (objectCounts_.count("Document") && objectCounts_.at("Document") > 1)
            {
                std::cout << "\nDOCUMENT LEAK ANALYSIS:\n";
                std::cout << "- Multiple Document instances detected\n";
                std::cout << "- Check Document sharing between Editor instances\n";
                std::cout << "- Verify Document ownership semantics\n";
            }

            std::cout << "\nNEXT STEPS:\n";
            std::cout << "1. Add TRACK_MEMORY to all constructors (including copy/move)\n";
            std::cout << "2. Check object ownership in EditorApplication constructor\n";
            std::cout << "3. Verify all components use std::move where appropriate\n";
            std::cout << "4. Ensure UNTRACK_MEMORY is in destructors\n";
            std::cout << "======================\n\n";
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
        std::unordered_map<std::string, std::set<const void*>> objectPointers_;
        std::set<const void*> allTrackedPointers_;
        size_t totalObjects_ = 0;
    };
}


#define TRACK_MEMORY(type, ptr) \
        mexedit::utils::MemoryDebugger::getInstance().trackCreation(#type, ptr)

#define UNTRACK_MEMORY(type, ptr) \
        mexedit::utils::MemoryDebugger::getInstance().trackDestruction(#type, ptr)

#endif // MEXEDIT_MEMORY_DEBUGGER_H
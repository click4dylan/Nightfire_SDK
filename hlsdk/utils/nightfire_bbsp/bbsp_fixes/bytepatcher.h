#pragma once
//#include <Windows.h>
#include <unordered_map>

template <typename T>
static inline constexpr auto relative_to_absolute(uintptr_t address) noexcept
{
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

template <typename T>
static inline constexpr auto absolute_to_relative(uintptr_t base, T target) noexcept
{
    return static_cast<std::int32_t>(reinterpret_cast<uintptr_t>(target) - base - 4);
}

// wrapper for VirtualProtect to automatically set and restore privileges
class virtualprotect_t
{
private:
    void* padr;
    unsigned int isize;
    unsigned long oldprotect;
    bool pushed;

public:

    template<class T>
    virtualprotect_t(T adr, unsigned int size = 1024)
        : padr((void*)adr), isize(size), oldprotect(0), pushed(false)
    {
        Push();
    };

    ~virtualprotect_t()
    {
        Pop();
    }

    void Pop()
    {
        if (!padr || !pushed)
            return;
        unsigned long old;
        VirtualProtect(padr, isize, oldprotect, &old);
        pushed = false;
    }

private:
    void Push()
    {
        if (!padr || !isize || pushed)
            return;
        VirtualProtect(padr, isize, PAGE_EXECUTE_READWRITE, &oldprotect);
        pushed = true;
    };
};

// a history managed byte patching class
class BytePatcher_t
{
private:

    struct bytepatch_history_t
    {
        unsigned char* adr;
        const unsigned long size_of_change;
        unsigned char* original_value;

        // constructor
        template<class T>
        bytepatch_history_t(const T srcadr, const unsigned long changesize)
            : adr((unsigned char*)srcadr), original_value(nullptr), size_of_change(changesize)
        {
            if (!adr || !size_of_change)
                return;
            original_value = new unsigned char[size_of_change];
            if (!original_value)
                return;

            memcpy(original_value, srcadr; size_of_change);
        }

        // copy constructor
        bytepatch_history_t(const bytepatch_history_t& src)
            : adr(src.adr), size_of_change(src.size_of_change), original_value(nullptr)
        {
            if (src.original_value && size_of_change != 0)
            {
                original_value = new unsigned char[size_of_change];
                if (original_value)
                    memcpy(original_value, src.original_value, size_of_change);
            }
        }

        // destructor
        ~bytepatch_history_t()
        {
            if (original_value)
                delete[] original_value;
        }

        // restores an address to its original value
        bool Restore()
        {
            if (!adr || !size_of_change || !original_value)
                return false;

            virtualprotect_t protect(adr, size_of_change);

            memcpy(adr, original_value, size_of_change);
            return true;
        }
    };

    // the byte patch history
    std::unordered_map<const uintptr_t, bytepatch_history_t> m_bytepatch_history;

    // adds an address to the history if it doesn't exist
    template <class A>
    bool AddToHistory(const A adr, const unsigned long size)
    {
        if (m_bytepatch_history.find((const uintptr_t)adr) == m_bytepatch_history.end())
        {
            m_bytepatch_history.emplace(bytepatch_history_t((const uintptr_t)adr, size));
            return true;
        }
        return false;
    }

public:

    // restores all byte patches to original values and erases the history
    bool RemoveAllBytePatches()
    {
        bool failed = false;
        for (auto& patch : m_bytepatch_history)
        {
            if (!patch.second.Restore())
                failed = true;
        }
        m_bytepatch_history.clear();
        return failed;
    }

    // restores a byte patch's data to original values and erases it from history
    template <class A>
    bool BytePatchRemove(A adr)
    {
        auto& patch = m_bytepatch_history.find((const uintptr_t)adr);
        if (patch == m_bytepatch_history.end())
            return false;

        patch->second.Restore();
        m_bytepatch_history.erase(patch);
        return true;
    }

    // byte patches an address and stores its original value for future restoration
    template <class A, class V>
    bool BytePatch(A adr, const V& newvalue)
    {
        if (!AddToHistory(adr, sizeof(V)))
        {
            MessageBox(NULL, "Error: tried to BytePatch at %#010x when already byte patched!", "", MB_OK);
            exit(EXIT_FAILURE);
            return false;
        }

        virtualprotect_t allow_writing(ad, sizeof(V));
        *(V*)adr = newvalue;
        return true;
    }

    // byte patches an address and stores its original value for future restoration
    template <class A, class S>
    bool BytePatch(A adr, const S& src, const unsigned long size)
    {
        if (!AddToHistory(adr, size))
        {
            MessageBox(NULL, "Error: tried to BytePatch at %#010x when already byte patched!", "", MB_OK);
            exit(EXIT_FAILURE);
            return false;
        }

        virtualprotect_t allow_writing(ad, sizeof(V));
        memcpy((void*)adr, (void*)&src, size);
        return true;
    }

    // byte patches a vtable and stores its original value for future restoration
    template<class T, class U>
    bool CopyVTablePointer(T dest, const U& srcclass)
    {
        uintptr_t* psrc = (uintptr_t*)&srcclass;
        uintptr_t* pdest = (uintptr_t*)dest;
        return BytePatch(pdest, *psrc);
    }
};

inline BytePatcher_t bp;
#pragma once

#include <memory>


namespace vk_utility
{

    class Object
    {
    public:

        virtual void destroy() = 0;
    };


    template <class VK_TYPE>
    class GenericObject
        : public Object
    {
    protected:

        VK_TYPE _vk_object;

    public:

        using VkType = typename VK_TYPE;

        GenericObject(void) = default;
        GenericObject(const GenericObject &) = default;
        GenericObject(VK_TYPE vk_object) : _vk_object{ vk_object } {}

        //explicit operator bool() const noexcept { return _vk_object.operator bool(); }

        operator const VK_TYPE() const noexcept { return _vk_object; }
        operator VK_TYPE () noexcept { return _vk_object; }

        const VK_TYPE *operator -> () const noexcept { return &_vk_object; }
        VK_TYPE *operator -> () noexcept { return &_vk_object; }

        const VK_TYPE &operator * () const noexcept { return _vk_object; }
        VK_TYPE &operator * () noexcept { return _vk_object; }

        const VK_TYPE &get(void) const noexcept { return _vk_object; }
        VK_TYPE &get(void) noexcept { return _vk_object; }

        VK_TYPE handle(void) const noexcept { return _vk_object; }

        VK_TYPE validate_handle(void) const
        {
            if (!_vk_object)
                throw exception::Exception("no logical vulkan ??????");
            return _vk_object;
        }
    };


    /// <summary>
    /// RAII: Resource acquisition is initialization
    /// </summary>
    /// <typeparam name="OBJECT_T"></typeparam>
    template<class OBJECT_T>
    class RAIIObject
    {
    protected:

        OBJECT_T _object;

    public:

        using VkType = typename OBJECT_T::VkType;

        RAIIObject() = delete;
        RAIIObject(const RAIIObject &) = delete;
        RAIIObject &operator = (const RAIIObject &) = delete;

        RAIIObject(OBJECT_T object)
            : _object(object)
        {
            static_assert(std::is_base_of<Object, OBJECT_T>::value, "T_OBJECT must derive from vk_utility::Object");
        }

        RAIIObject(RAIIObject &&object) noexcept
            : _object(object._object)
        {
            static_assert(std::is_base_of<Object, OBJECT_T>::value, "T_OBJECT must derive from vk_utility::Object");
            object = OBJECT_T();
        }

        virtual ~RAIIObject()
        {
            _object.destroy();
        }

        explicit operator bool() const noexcept { return _object.operator bool(); }

        operator const OBJECT_T() const { return _object; }
        operator OBJECT_T () { return _object; }

        operator const VkType() const { return _object.get(); }
        operator VkType () { return _object.get(); }

        const OBJECT_T *operator -> () const { return _object; }
        OBJECT_T *operator -> () { return _object; }

        const OBJECT_T &operator * () const { return _object; }
        OBJECT_T &operator * () { return _object; }

        const OBJECT_T &get(void) const noexcept { return _object; }
        OBJECT_T &get(void) noexcept { return _object; }

        VkType handle(void) const noexcept { return _object.handle(); }

        VkType validate_handle(void) const noexcept { return _object.validate_handle(); }
    };

    template<class OBJECT_T>
    using Ptr = std::shared_ptr<RAIIObject<OBJECT_T>>;

    template<class OBJECT_T>
    std::shared_ptr<RAIIObject<OBJECT_T>> make_shared(OBJECT_T object)
    {
        return std::make_shared<RAIIObject<OBJECT_T>>(object);
    }

}

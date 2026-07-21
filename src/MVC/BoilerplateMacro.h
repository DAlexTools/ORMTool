#pragma once 


/**
 * @brief Implements typed model accessors for IControllerTyped<TModel>.
 * @param Type Model type stored by the controller.
 * @param Field std::shared_ptr<Type> member that owns the model.
 */
#define IMPL_TYPED_MODEL(Type, Field) \
public:\
    std::shared_ptr<Type> GetTypedModel() override { return Field; } \
    std::shared_ptr<const Type> GetTypedModel() const override { return Field; }

/**
 * @brief Implements typed controller accessors for IViewTyped<TController>.
 * @param Type Controller type stored by the view.
 * @param Field std::shared_ptr<Type> member that owns the controller.
 */
#define IMPL_TYPED_CONTROLLER(Type, Field) \
public:\
    std::shared_ptr<Type> GetTypedController() override { return Field; } \
    std::shared_ptr<const Type> GetTypedController() const override { return Field; }

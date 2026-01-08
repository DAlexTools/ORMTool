#pragma once 


/**
 * Macro: IMPL_TYPED_MODEL
 *
 * Implements the GetModel() and GetModel() const methods required by the IControllerTyped<TModel> interface.
 * This macro eliminates boilerplate in controller classes that expose access to their model via std::shared_ptr.
 *
 * Parameters:
 *   Type  - The model type (e.g. MenuBarModel, NoisePropertyModel)
 *   Field - A std::shared_ptr<Type> member variable inside the class
 *
 * Usage Example:
 *
 * class NoisePropertyController : public IControllerTyped<NoisePropertyModel>
 * {
 * public:
 *     NoisePropertyController()
 *         : Model(std::make_shared<NoisePropertyModel>()) {}
 *
 *     IMPL_TYPED_MODEL(NoisePropertyModel, Model)
 *
 * private:
 *     std::shared_ptr<NoisePropertyModel> Model;
 * };
 *
 * Notes:
 * - This macro satisfies both const and non-const overloads of GetModel().
 * - Works seamlessly with IControllerTyped<TModel>::GetModelRef().
 * - Keeps access uniform across all controllers using shared ownership of models.
 */
#define IMPL_TYPED_MODEL(Type, Field) \
public:\
    std::shared_ptr<Type> GetModel() override { return Field; } \
    std::shared_ptr<const Type> GetModel() const override { return Field; }

 /**
  * Macro: IMPL_TYPED_CONTROLLER
  *
  * Implements the GetTypedController() and GetTypedController() const methods required by the IViewTyped<TController> interface.
  * Useful in view classes to expose their strongly-typed controller via std::shared_ptr.
  *
  * Parameters:
  *   Type  - The controller type (e.g. NoisePanelController)
  *   Field - A std::shared_ptr<Type> member variable inside the class
  *
  * Usage Example:
  *
  * class NoisePreviewPanelUI : public IView, public IViewTyped<NoisePanelController>
  * {
  * public:
  *     NoisePreviewPanelUI()
  *         : Controller(std::make_shared<NoisePanelController>()) {}
  *
  *     IMPL_TYPED_CONTROLLER(NoisePanelController, Controller)
  *
  * private:
  *     std::shared_ptr<NoisePanelController> Controller;
  * };
  *
  * Notes:
  * - Allows safe access to controller via `GetControllerRef()` or `GetTypedController()`.
  * - Must be used with std::shared_ptr-based ownership.
  * - Helps enforce architectural clarity between UI and logic layers.
  */
#define IMPL_TYPED_CONTROLLER(Type, Field) \
public:\
    std::shared_ptr<Type> GetTypedController() override { return Field; } \
    std::shared_ptr<const Type> GetTypedController() const override { return Field; }
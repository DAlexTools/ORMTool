#pragma once
#include "IModel.h"
#include <memory>

/**
 * @brief Base interface for controllers in the MVC layer.
 */
class IController
{
public:
	/**
	 * @brief Destroys the controller through the base interface.
	 */
	virtual ~IController() = default;

	/**
	 * @brief Initializes controller state, bindings, and event handlers.
	 */
	virtual void Initialize() = 0;
};

/**
 * @brief Typed extension for controllers that expose a strongly typed model.
 * @tparam TModel Model type managed by this controller.
 */
template <typename TModel> class IControllerTyped
{
public:
	/**
	 * @brief Destroys the typed controller through the base interface.
	 */
	virtual ~IControllerTyped() = default;

	/**
	 * @brief Returns a shared pointer to the associated model.
	 * @return Mutable model pointer.
	 */
	virtual std::shared_ptr<TModel> GetTypedModel() = 0;

	/**
	 * @brief Returns a read-only shared pointer to the associated model.
	 * @return Const model pointer.
	 */
	virtual std::shared_ptr<const TModel> GetTypedModel() const = 0;

	/**
	 * @brief Returns a direct mutable reference to the associated model.
	 * @return Mutable model reference.
	 */
	TModel& GetModelRef()
	{
		return *GetTypedModel();
	}

	/**
	 * @brief Returns a direct read-only reference to the associated model.
	 * @return Const model reference.
	 */
	const TModel& GetModelRef() const
	{
		return *GetTypedModel();
	}
};

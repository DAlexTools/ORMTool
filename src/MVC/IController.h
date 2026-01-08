#pragma once 
#include <memory>
#include "IModel.h"

/**
 * Interface: IController
 *
 * Base interface for all controller classes in the MVC architecture.
 * Controllers act as intermediaries between UI views and data models,
 * providing business logic, event handling, and state management.
 *
 * Notes:
 * - All controllers must implement Initialize() to prepare internal state or bind events.
 * - Typically used in combination with IControllerTyped<TModel>.
 */
class IController
{
public:
	virtual ~IController() = default;

	/** Called once to initialize controller state, bind data, or set up handlers. */
	virtual void Initialize() = 0;
};
class IController
{
public:
	virtual ~IController() = default;
	virtual void Initialize() = 0;
};



/**
 * Interface: IControllerTyped
 *
 * Typed extension of the base controller interface for exposing an associated model.
 * Used to provide safe, structured access to a strongly-typed data model instance.
 *
 * Template Parameter:
 *   TModel - The specific model class managed by this controller.
 *
 * Notes:
 * - Both const and non-const shared pointer accessors must be implemented.
 * - Provides convenient GetModelRef() methods for reference-style access.
 */
template<typename TModel>
class IControllerTyped
{
public:
	virtual ~IControllerTyped() = default;

	/** Returns a mutable shared pointer to the associated model. */
	virtual std::shared_ptr<TModel> GetTypedModel() = 0;

	/** Returns a read-only shared pointer to the associated model. */
	virtual std::shared_ptr<const TModel> GetTypedModel() const = 0;

	/** Returns a direct mutable reference to the associated model. */
	TModel& GetModelRef()
	{
		return *GetModel();
	}

	/** Returns a direct read-only reference to the associated model. */
	const TModel& GetModelRef() const
	{
		return *GetModel();
	}
};
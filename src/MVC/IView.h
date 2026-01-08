#pragma once 
#include <memory>
#include "IController.h"


/**
 * Interface: IView
 *
 * Base interface for all UI view classes in the MVC architecture.
 * Represents a visual component that is responsible for rendering
 * and managing user-facing interface elements.
 *
 * Notes:
 * - Initialize() is called once to prepare internal state and bind the controller.
 * - Draw() is called every frame to render the UI contents.
 */
class IView
{
public:
	virtual ~IView() = 0;

	virtual void Initialize() = 0;
	virtual void Draw() = 0;
};


/**
 * Interface: IViewTyped
 *
 * Typed extension of the base view interface for exposing a strongly-typed controller.
 * Enables views to safely access their controller logic with full type information.
 *
 * Template Parameter:
 *   TController - The specific controller class used by this view.
 *
 * Notes:
 * - Both const and non-const accessors must be implemented.
 * - Helper methods GetControllerRef() and GetControllerRef() const provide
 *   direct reference access for convenience and improved readability.
 */
template<typename TController>
class IViewTyped
{
public:
	virtual ~IViewTyped() = default;

	/** Returns a shared pointer to the controller instance. */
	virtual std::shared_ptr<TController> GetTypedController() = 0;

	/** Returns a shared pointer to the controller instance (const version). */
	virtual std::shared_ptr<const TController> GetTypedController() const = 0;

	/** Returns a direct reference to the controller instance. */
	TController& GetControllerRef()
	{
		return *GetTypedController();
	}

	/** Returns a direct const reference to the controller instance. */
	const TController& GetControllerRef() const
	{
		return *GetTypedController();
	}
};


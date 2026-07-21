#pragma once 
#include <memory>
#include "IController.h"


/**
 * @brief Base interface for UI views in the MVC layer.
 */
class IView
{
public:
	/**
	 * @brief Destroys the view through the base interface.
	 */
	virtual ~IView() = 0;

	/**
	 * @brief Prepares the view before it is drawn.
	 */
	virtual void Initialize() = 0;

	/**
	 * @brief Draws the view for the current UI frame.
	 */
	virtual void Draw() = 0;
};


/**
 * @brief Typed extension for views that expose a strongly typed controller.
 * @tparam TController Controller type used by this view.
 */
template<typename TController>
class IViewTyped
{
public:
	/**
	 * @brief Destroys the typed view through the base interface.
	 */
	virtual ~IViewTyped() = default;

	/**
	 * @brief Returns a shared pointer to the controller instance.
	 * @return Mutable controller pointer.
	 */
	virtual std::shared_ptr<TController> GetTypedController() = 0;

	/**
	 * @brief Returns a read-only shared pointer to the controller instance.
	 * @return Const controller pointer.
	 */
	virtual std::shared_ptr<const TController> GetTypedController() const = 0;

	/**
	 * @brief Returns a direct mutable reference to the controller instance.
	 * @return Mutable controller reference.
	 */
	TController& GetControllerRef()
	{
		return *GetTypedController();
	}

	/**
	 * @brief Returns a direct read-only reference to the controller instance.
	 * @return Const controller reference.
	 */
	const TController& GetControllerRef() const
	{
		return *GetTypedController();
	}
};


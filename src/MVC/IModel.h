#pragma once
#include <string>

/**
 * @brief Base interface for data models used by the MVC layer.
 */
class IModel
{
public:
	/**
	 * @brief Destroys the model through the base interface.
	 */
	virtual ~IModel() = default;

	/**
	 * @brief Returns a unique model identifier.
	 * @return Model id string.
	 */
	virtual std::string GetId() const = 0;

	/**
	 * @brief Restores the model's internal state to default values.
	 */
	virtual void Reset() = 0;
};

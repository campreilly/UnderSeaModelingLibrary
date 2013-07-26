/**
 * @file lockingvalue.h
 */

#pragma once

#include <pthreadcc.h>

/**
 * @class LockingValue
 * @brief A thread-safe templated value to use between threads.
 */
template <class Type>
class LockingValue
{
public:

	/**
	 * Default constructor.  Sets the value to 0.
	 */
	LockingValue()
		: m_pclMutex(NULL), m_clValue(0)
	{
		m_pclMutex = new MutualExclusion();
	}
	
	/**
	 * Constructor.
	 * @param clValue The starting value.
	 */
	LockingValue(const Type clValue)
		: m_pclMutex(NULL), m_clValue(clValue)
	{
		m_pclMutex = new MutualExclusion();
	}
	
	/**
	 * Copy constructor.
	 * @param clLockingValue The locking value to copy.
	 */
	LockingValue(const LockingValue& clLockingValue)
		: m_clValue(clLockingValue.GetValue())
	{
		m_pclMutex = new MutualExclusion();
	}
	
	/**
	 * Destructor.
	 */
	~LockingValue()
	{
		if (m_pclMutex)
		{
			delete m_pclMutex;
			m_pclMutex = NULL;
		}
	}
	
	/**
	 * Assignment operator.
	 * @param clLockingValue The locking value to copy.
	 * @return The newly copied locking value.
	 */
	LockingValue& operator=(const LockingValue& clLockingValue)
	{
		// Do not re-assign the mutex.
		m_clValue = clLockingValue.GetValue();
	}
	
	/**
	 * Gets the value.
	 * @return The value.
	 */
	Type GetValue() const
	{
		Type clValue;
		
		m_pclMutex->Lock();
		
		clValue = m_clValue;
		
		m_pclMutex->Unlock();
		
		return clValue;
	}
	
	/**
	 * Sets the value.
	 * @param clValue The value to set.
	 */
	void SetValue(const Type clValue)
	{
		m_pclMutex->Lock();
		
		m_clValue = clValue;
		
		m_pclMutex->Unlock();
	}
	
	/**
	 * Increments the value.
	 * @param clIncrementValue The value to add to the current value.
	 * @return The newly incremented value.
	 */
	Type Increment(const Type clIncrementValue)
	{
		Type clValue;

		m_pclMutex->Lock();
		
		m_clValue += clIncrementValue;
		clValue = m_clValue;
		
		m_pclMutex->Unlock();
		
		return clValue;
	}
	
	/**
	 * Decrements the value.
	 * @param clDecrementValue The value to subtract from the current value.
	 * @return The newly decremented value.
	 */
	Type Decrement(const Type clDecrementValue)
	{
		Type clValue;
	
		m_pclMutex->Lock();
		
		m_clValue -= clDecrementValue;
		clValue = m_clValue;

		m_pclMutex->Unlock();
		
		return clValue;
	}
	
private:

	/** Mutex to prevent simultaneous access by multiple threads. */
	MutualExclusion* m_pclMutex;
	
	/** The value we are encapsulating. */
	Type m_clValue;
};


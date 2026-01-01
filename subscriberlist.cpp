/***********************************************************
Description: SubscriberList class
Author: Gandhar Deshpande
Date: 3/7/2023
************************************************************/

#include "subscriberlist.h"

void SubscriberList::addSubscriber(Subscriber *subscriber)
{
/**
 * Add a subscriber to the subscriber list
 * 
 * @param subscriber Pointer to the Subscriber instance to add to the list
 */
    subscribers.push_back(subscriber);
}

void SubscriberList::removeSubscriber(Subscriber *subscriber)
{
/**
 * Remove a subscriber from the subscriber list
 * Removes all instances of the subscriber if duplicates exist
 * 
 * @param subscriber Pointer to the Subscriber instance to remove from the list
 */
    subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriber), subscribers.end());
}

std::vector<Subscriber *> SubscriberList::getSubscribers() const
{
/**
 * Get a copy of the current subscriber list
 * 
 * @return std::vector<Subscriber*> Copy of the subscribers vector
 */
    return subscribers;
}

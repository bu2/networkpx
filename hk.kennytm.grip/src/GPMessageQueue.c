/*

FILE_NAME ... DESCRIPTION
 
Copyright (c) 2009, KennyTM~
All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the KennyTM~ nor the names of its contributors may be
   used to endorse or promote products derived from this software without
   specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

#include <GriP/GPMessageQueue.h>
#include <GriP/GPPreferences.h>
#include <GriP/common.h>
#include <GriP/GPSingleton.h>

static CFMutableArrayRef messageQueues[5] = {NULL, NULL, NULL, NULL, NULL};
static CFNotificationSuspensionBehavior gamingSuspensionBehaviors[5];
static CFNotificationSuspensionBehavior lockedSuspensionBehaviors[5];
static Boolean isGaming = false, isLocked = false;

CFNotificationSuspensionBehavior GPCurrentSuspensionBehaviorForPriorityIndex(int i) {
	return isLocked ? lockedSuspensionBehaviors[i] : isGaming ? gamingSuspensionBehaviors[i] : CFNotificationSuspensionBehaviorDeliverImmediately;
}

static CFNotificationSuspensionBehavior GPGetBehaviorFromObject(const void* x) {
	if (CFGetTypeID(x) == CFStringGetTypeID())
		return CFStringGetIntValue(x);
	else if (CFGetTypeID(x) == CFNumberGetTypeID()) {
		int retval;
		CFNumberGetValue(x, kCFNumberIntType, &retval);
		return retval;
	} else
		return CFNotificationSuspensionBehaviorHold;
}

void GPRefreshSuspensionBehaviors() {
	CFDictionaryRef prefs = GPCopyPreferences();
	CFArrayRef pps = CFDictionaryGetValue(prefs, CFSTR("PerPrioritySettings"));
	for (unsigned i = 0; i < 5; ++ i) {
		CFArrayRef ps = CFArrayGetValueAtIndex(pps, i);
		gamingSuspensionBehaviors[i] = GPGetBehaviorFromObject(CFArrayGetValueAtIndex(ps, GPPrioritySettings_Gaming));
		lockedSuspensionBehaviors[i] = GPGetBehaviorFromObject(CFArrayGetValueAtIndex(ps, GPPrioritySettings_Locked));
	}
	CFRelease(prefs);
}

extern void GPCleanUpSuspensionQueues() {
	// this is kinda finalizer, and will only be called by the finalizers.
	for (int i = 0; i < 5; ++ i)
		if (messageQueues[i] != NULL)
			CFRelease(messageQueues[i]);
}

CFArrayRef GPEnqueueMessage(CFDictionaryRef message) {
	int priorityIndex = 0;
	CFNumberGetValue(CFDictionaryGetValue(message, GRIP_PRIORITY), kCFNumberIntType, &priorityIndex);
	priorityIndex += 2;
	
	CFNotificationSuspensionBehavior currentSuspensionBehavior = GPCurrentSuspensionBehaviorForPriorityIndex(priorityIndex);
	if (currentSuspensionBehavior == CFNotificationSuspensionBehaviorDrop) {
		return CFArrayCreate(NULL, (const void**)&message, 1, &kCFTypeArrayCallBacks);
	}
	
	GPSingletonConstructor(messageQueues[priorityIndex],
						   __NEWOBJ__ = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks),
						   if(__NEWOBJ__ != NULL) CFRelease(__NEWOBJ__));
	
	if (currentSuspensionBehavior == CFNotificationSuspensionBehaviorCoalesce) {
		CFArrayRef queues = messageQueues[priorityIndex];
		CFMutableArrayRef coalescedQueue = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(coalescedQueue, (const void*)message);
		if (!OSAtomicCompareAndSwapPtrBarrier((void*)queues, (void*)coalescedQueue, (void*volatile*)&(messageQueues[priorityIndex])))
			CFRelease(coalescedQueue);
		return queues;
		
	} else {
		CFArrayAppendValue(messageQueues[priorityIndex], message);
		return NULL;
	}
}

// TODO: Thread safety check.
CFArrayRef GPCopyAndDequeueMessages(unsigned maxCount) {
	if (maxCount == 0)
		maxCount = UINT_MAX;
	
	CFMutableArrayRef resArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	for (int i = 4; i >= 0; --i) {
		if (messageQueues[i] == NULL)
			continue;
		
		if (GPCurrentSuspensionBehaviorForPriorityIndex(i) == CFNotificationSuspensionBehaviorDeliverImmediately) {
			CFIndex count = CFArrayGetCount(messageQueues[i]);
			if (count == 0)
				continue;
			
			if (count < maxCount) {
				CFArrayAppendArray(resArray, messageQueues[i], CFRangeMake(0, count)); 
				CFArrayRemoveAllValues(messageQueues[i]);
				maxCount -= count;
			} else {
				CFIndex delta = count - maxCount;
				CFArrayAppendArray(resArray, messageQueues[i], CFRangeMake(0, delta));
				for (CFIndex j = delta-1;; -- j) {
					CFArrayRemoveValueAtIndex(messageQueues[i], j);
					if (j == 0) break;
				}
			}
		}
	}
	
	return resArray;
}

void GPSetLocked(Boolean locked) { isLocked = locked; }
void GPSetGaming(Boolean gaming) { isGaming = gaming; }
Boolean GPGetLocked() { return isLocked; }
//
// Created by hungpm on 24/03/2024.
//

#include "Enclave/tasks.h"
#include "Enclave/Enclave.h"
#include "fast_call.h"
#include "StreamBox/schemas.h"
#include "sgx_trts_exception.h"

#include <map>
#include <string>
#include <stdexcept>

using namespace std;

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;
char encryptedData[1000];

std::vector<TripData*> trips;
const uint64_t tripWindowSize = 5500;const uint64_t tripSlidingStep = 4100;
// const uint64_t tripWindowSize = 5000;
// const uint64_t tripSlidingStep = 100;
uint64_t tripCurrentSliding = tripSlidingStep;
void MaxTrip(void* data) {
    if (data == NULL) {
        return;
    }

    const auto tripData = static_cast<TripData*>(data);

//     if window size reached --> delete old data
    if (trips.size() >= tripWindowSize) {
        delete trips[0];
        trips.erase(trips.begin());
    }

    // add new data
    auto* newTrip = new TripData;
    strncpy(newTrip->medallion, tripData->medallion, 64);
    strncpy(newTrip->hackLicense, tripData->hackLicense, 64);
    strncpy(newTrip->pickupDateTime, tripData->pickupDateTime, 64);
    strncpy(newTrip->dropOffDateTime, tripData->dropOffDateTime, 64);
    newTrip->tripTimeInSecs = tripData->tripTimeInSecs;
    newTrip->tripDistance = tripData->tripDistance;
    newTrip->pickupLongitude = tripData->pickupLongitude;
    newTrip->pickupLatitude = tripData->pickupLatitude;
    newTrip->dropOffLongitude = tripData->dropOffLongitude;
    newTrip->dropOffLatitude = tripData->dropOffLatitude;

    trips.push_back(newTrip);

    if (trips.size() < tripWindowSize) {
        return;
    }

    tripCurrentSliding++;

    // if sliding enough --> count using a hash table<auction_id, count>
    if (tripCurrentSliding >= tripSlidingStep) {
        tripCurrentSliding = 0;

        TripData* maxData = trips[0];
        for (auto& storedTrip : trips) {
            if (storedTrip->tripTimeInSecs > maxData->tripTimeInSecs) {
                maxData = storedTrip;
            }
        }

        FastCall_request_encrypt2(globalFastOCall, maxData, encryptedData);
    }
}
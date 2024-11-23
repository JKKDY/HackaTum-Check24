#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <iostream>

using namespace Pistache;
using json = nlohmann::json;

// Offer structure
struct Offer {
    std::string ID;
    std::string data;
    int mostSpecificRegionID;
    int64_t startDate;
    int64_t endDate;
    int numberSeats;
    int price;
    std::string carType;
    bool hasVollkasko;
    int freeKilometers;

    // Parse Offer from JSON
    static Offer fromJson(const json& j) {
        return Offer{
                j.at("ID").get<std::string>(),
                j.at("data").get<std::string>(),
                j.at("mostSpecificRegionID").get<int>(),
                j.at("startDate").get<int64_t>(),
                j.at("endDate").get<int64_t>(),
                j.at("numberSeats").get<int>(),
                j.at("price").get<int>(),
                j.at("carType").get<std::string>(),
                j.at("hasVollkasko").get<bool>(),
                j.at("freeKilometers").get<int>()
        };
    }
};

// Global map to store offers
std::unordered_map<std::string, Offer> offers;

class OffersHandler {
public:
    // POST /api/offers - Adds new offers
    static void postOffers(const Rest::Request& request, Http::ResponseWriter response) {
        try {
            // Parse JSON body
            auto body = json::parse(request.body());
            if (!body.contains("offers") || !body["offers"].is_array()) {
                response.send(Http::Code::Bad_Request, "Invalid JSON body");
                return;
            }

            // Process and store offers
            for (const auto& offerJson : body["offers"]) {
                Offer offer = Offer::fromJson(offerJson);
                offers[offer.ID] = offer;  // Store the offer by ID
            }

            response.send(Http::Code::Ok, "Offers added successfully");
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
    }

    // GET /api/offers - Retrieves all offers
    static void getOffers(const Rest::Request& request, Http::ResponseWriter response) {
        // Serialize offers to JSON
        json responseBody = {{"offers", json::array()}};
        for (const auto& [id, offer] : offers) {
            responseBody["offers"].push_back({
                                                     {"ID", offer.ID},
                                                     {"data", offer.data},
                                                     {"mostSpecificRegionID", offer.mostSpecificRegionID},
                                                     {"startDate", offer.startDate},
                                                     {"endDate", offer.endDate},
                                                     {"numberSeats", offer.numberSeats},
                                                     {"price", offer.price},
                                                     {"carType", offer.carType},
                                                     {"hasVollkasko", offer.hasVollkasko},
                                                     {"freeKilometers", offer.freeKilometers}
                                             });
        }

        response.send(Http::Code::Ok, responseBody.dump());
    }

    // DELETE /api/offers - Clears all offers
    static void deleteOffers(const Rest::Request& request, Http::ResponseWriter response) {
        offers.clear();
        response.send(Http::Code::Ok, "All offers deleted");
    }
};

// Set up routes
void setupRoutes(Rest::Router& router) {
    Rest::Routes::Post(router, "/api/offers", Rest::Routes::bind(&OffersHandler::postOffers));
    Rest::Routes::Get(router

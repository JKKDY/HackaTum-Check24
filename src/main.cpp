#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>  // For JSON parsing
#include "src/DataBase.h"

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

    // Simple constructor to initialize the Offer object
    Offer(const std::string& id, const std::string& data, int regionID, int64_t start, int64_t end,
          int seats, int price, const std::string& carType, bool vollkasko, int freeKm)
            : ID(id), data(data), mostSpecificRegionID(regionID), startDate(start), endDate(end),
              numberSeats(seats), price(price), carType(carType), hasVollkasko(vollkasko), freeKilometers(freeKm) {}
};

// In-memory list to store offers
std::vector<Offer> offers;

// Handler for the Offers API
class OffersHandler {
public:
    // POST /api/offers - Adds new offers
    static void postOffers(const Rest::Request& request, Http::ResponseWriter response) {
        try {
            // Parse the JSON body of the request
            auto body = json::parse(request.body());
            if (body.find("offers") == body.end()) {
                response.send(Http::Code::Bad_Request, "Missing 'offers' field in JSON");
                return;
            }

            // Process each offer in the 'offers' array
            const auto& offersArray = body["offers"];
            for (const auto& offerJson : offersArray) {
                Offer offer(
                        offerJson["ID"].get<std::string>(),
                        offerJson["data"].get<std::string>(),
                        offerJson["mostSpecificRegionID"].get<int>(),
                        offerJson["startDate"].get<int64_t>(),
                        offerJson["endDate"].get<int64_t>(),
                        offerJson["numberSeats"].get<int>(),
                        offerJson["price"].get<int>(),
                        offerJson["carType"].get<std::string>(),
                        offerJson["hasVollkasko"].get<bool>(),
                        offerJson["freeKilometers"].get<int>()
                );

                // Add the offer to the in-memory vector
                offers.push_back(offer);
            }
            db.add_offer(offer)
            response.send(Http::Code::Ok, "Offers added successfully");
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
    }
};

void setupRoutes(Rest::Router& router) {
    Rest::Routes::Post(router, "/api/offers", Rest::Routes::bind(&OffersHandler::postOffers));
}

int main() {
    Database db;
    Address addr(Ipv4::any(), Port(80));
    auto opts = Http::Endpoint::options().threads(1);

    Http::Endpoint server(addr);
    server.init(opts);

    Rest::Router router;
    setupRoutes(router);

    server.setHandler(router.handler());
    std::cout << "Server running on port 80..." << std::endl;
    server.serve();

    return 0;
}



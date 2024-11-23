#include <iostream>
#include <nlohmann/json.hpp> // For JSON parsing
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <string>

#include "database/DataBase.h"

using json = nlohmann::json;


using namespace db;

CarType car_type_from_string(const std::string &car_type_str) {
	if (car_type_str == "small")
		return CarType::SMALL;
	if (car_type_str == "sports")
		return CarType::SPORTS;
	if (car_type_str == "luxury")
		return CarType::LUXURY;
	if (car_type_str == "family")
		return CarType::FAMILY;
	return CarType::ALL; // Default for unknown car types
}


// Handler for the Offers API
class OffersHandler {
public:
	OffersHandler(db::DataBase db) : database(db) {}

	// POST /api/offers - Adds new offers
	void postOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {
		std::cout << "OffersHandler::postOffers" << std::endl;
		try {
			// Parse the JSON body of the request
			auto body = json::parse(request.body());
			if (body.find("offers") == body.end()) {
				response.send(Pistache::Http::Code::Bad_Request, "Missing 'offers' field in JSON");
				return;
			}

			// Process each offer in the 'offers' array
			const auto &offersArray = body["offers"];
			for (const auto &offerJson : offersArray) {
				Offer offer ={
						.start_date = offerJson["startDate"].get<int64_t>(),
						.end_date = offerJson["endDate"].get<int64_t>(),
						.id = offerJson["ID"].get<std::string>(),
						.data =
							[&offerJson] {
								std::array<char, 256> arr{};
								std::string data_str = offerJson["data"].get<std::string>();
								std::copy(data_str.begin(), data_str.end(), arr.begin());
								return arr;
							}(),
						.region_id = offerJson["mostSpecificRegionID"].get<int>(),
						.number_seats = offerJson["numberSeats"].get<int>(),
						.price = offerJson["price"].get<int>(),
						.free_kilometers = offerJson["freeKilometers"].get<int>(),
						.car_type = car_type_from_string(offerJson["carType"].get<std::string>()),
						.has_vollkasko = offerJson["hasVollkasko"].get<bool>()
					};

				std::cout << "New offer: "<<offer.id << std::endl;
				database.add_offer(offer);
				// Add the offer to the in-memory vector
			}

			response.send(Pistache::Http::Code::Ok, "Offers added successfully");
		}
		catch (const std::exception &e) {
			response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
		}
	}


    // GET /api/offers - Adds new offers
    void getOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {
        std::cout << "OffersHandler::getOffers" << std::endl;

        try {
            auto body = json::parse(request.body());
            // Parse the required fields
            GetRequest get_request = {
                    .region_id = body.at("region_id").get<int>(),  // Required
                    .time_range_start = body.at("time_range_start").get<int64_t>(),  // Required
                    .time_range_end = body.at("time_range_end").get<int64_t>(),  // Required
                    .number_days = body.at("number_days").get<int>(),  // Required

                    .sort_order = body.value("sort_order", SortOrder::ASCENDING),  // Optional with default
                    .page = body.value("page", 1),  // Optional with default
                    .page_size = body.value("page_size", 10),  // Optional with default
                    .price_range_width = body.value("price_range_width", 1000),  // Optional with default
                    .min_free_kilometer_width = body.value("min_free_kilometer_width", 100),  // Optional with default

                    .min_number_seats = body.value("min_number_seats", 0),  // Optional
                    .min_price = body.value("min_price", 0),  // Optional
                    .max_price = body.value("max_price", 0),  // Optional
                    .car_type = body.value("car_type", CarType::ALL),  // Optional
                    .only_vollkasko = body.value("only_vollkasko", false),  // Optional
                    .min_free_kilometer = body.value("min_free_kilometer", 0)  // Optional
            };

            Offers offers = database.get(get_request);

            // Convert the Offers object to JSON
            json response_json = {
                    {"offers", json::array()},
                    {"price_ranges", json::array()},
                    {"car_type_counts", {
                                       {"small_count", offers.car_type_counts.small_count},
                                       {"sports_count", offers.car_type_counts.sports_count},
                                       {"luxury_count", offers.car_type_counts.luxury_count},
                                       {"family_count", offers.car_type_counts.family_count}
                               }},
                    {"seat_counts", json::array()},
                    {"free_kilometer_ranges", json::array()},
                    {"voll_kasko_count", {
                                       {"vollkasko_true_count", offers.voll_kasko_count.vollkasko_true_count},
                                       {"vollkasko_false_count", offers.voll_kasko_count.vollkasko_false_count}
                               }}
            };

            // Serialize the `offers` field
            for (const auto& offer : offers.offers) {
                response_json["offers"].push_back({
                                                          {"id", offer.id},
                                                          {"data", std::string(offer.data.begin(), offer.data.end())}  // Convert array<char, 256> to string
                                                  });
            }

            // Serialize the `price_ranges` field
            for (const auto& range : offers.price_ranges) {
                response_json["price_ranges"].push_back({
                                                                {"price_range_start", range.price_range_start},
                                                                {"price_range_end", range.price_range_end},
                                                                {"price_range_count", range.price_range_count}
                                                        });
            }

            // Serialize the `seat_counts` field
            for (const auto& seat : offers.seat_counts) {
                response_json["seat_counts"].push_back({
                                                               {"number_seats", seat.number_seats},
                                                               {"count", seat.count}
                                                       });
            }

            // Serialize the `free_kilometer_ranges` field
            for (const auto& range : offers.free_kilometer_ranges) {
                response_json["free_kilometer_ranges"].push_back({
                                                                         {"free_kilometer_range_start", range.free_kilometer_range_start},
                                                                         {"free_kilometer_range_end", range.free_kilometer_range_end},
                                                                         {"free_kilometer_range_count", range.free_kilometer_range_count}
                                                                 });
            }

            // Send the JSON response
            response.send(Pistache::Http::Code::Ok, response_json.dump());
        }
        catch (const json::exception& e) {
            // Handle JSON parsing errors or missing fields
            response.send(Pistache::Http::Code::Bad_Request, std::string("Error parsing request: ") + e.what());
        }
    }


	// DELETE /api/offers - Clears all offers
	void deleteOffers(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
		std::cout << "deleteOffers" << std::endl;
		try {
			// database.clear_offers(); // Assuming the database has a method to clear all offers
			std::cout << "All offers cleared from the database" << std::endl;
			response.send(Pistache::Http::Code::Ok, "Data was cleaned up");
		}
		catch (const std::exception& e) {
			response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
		}
	}



private:
	DataBase &database;
};


void setupRoutes(Pistache::Rest::Router &router, OffersHandler& offersHandler) {
	using namespace Pistache::Rest;
	Routes::Post(router, "/api/offers", Routes::bind(&OffersHandler::postOffers, &offersHandler));
	Routes::Delete(router, "/api/offers", Routes::bind(&OffersHandler::deleteOffers, &offersHandler));
    Routes::Get(router, "/api/offers", Routes::bind(&OffersHandler::getOffers, &offersHandler));
}

int main() {
	DataBase database;
	OffersHandler offersHandler(database);

	Pistache::Address address("*:80"); // Bind to all network interfaces on port 80
	Pistache::Http::Endpoint server(address);

	Pistache::Rest::Router router;
	setupRoutes(router, offersHandler);

    auto options = Pistache::Http::Endpoint::options()
    .threads(1)
    .flags(Pistache::Tcp::Options::ReuseAddr).maxRequestSize(100 * 1024 * 1024);  // Allow 100 MB payloads (you can adjust this as needed);

	server.init(options);

	server.setHandler(router.handler());

	std::cout << "Starting server on port 80..." << std::endl;

	server.serve();
	server.shutdown();

	return 0;
}

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


	void getOffer() {

		GetRequest request;

		offers = database.get(request);

		response.send(kdhglifdghjl)
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
	.flags(Pistache::Tcp::Options::ReuseAddr);

	server.init(options);

	server.setHandler(router.handler());

	std::cout << "Starting server on port 80..." << std::endl;

	server.serve();
	server.shutdown();

	return 0;
}

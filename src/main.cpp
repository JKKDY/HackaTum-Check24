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
	OffersHandler(db::DataBase & db) : database(db) {}

	// POST /api/offers - Adds new offers
	void postOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {
		std::cout << "POST" << std::endl;
		try {
			// Parse the JSON body of the request
			auto body = json::parse(request.body());
			if (body.find("offers") == body.end()) {
				response.send(Pistache::Http::Code::Bad_Request, "Missing 'offers' field in JSON");
				return;
			}
			std::cout << "Pre Parse" << std::endl;

			// Process each offer in the 'offers' array
			const auto &offersArray = body["offers"];
			for (const auto &offerJson : offersArray) {
				Offer offer = {.start_date = offerJson["startDate"].get<int64_t>(),
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
							   .has_vollkasko = offerJson["hasVollkasko"].get<bool>()};

				std::cout << "New offer added: " << offer.id << std::endl;
				database.add_offer(offer);
			}
			response.send(Pistache::Http::Code::Ok, "Offers added successfully");
			std::cout << "Done with adding offers" << std::endl;

		}
		catch (const std::exception &e) {
			std::cout << "eeeeeeeeeeeeee" << e.what() << std::endl;
			response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
		}
	}


	// GET /api/offers - Adds new offers
	void getOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {
		std::cout << "OffersHandler::getOffers" << std::endl;

		try {
			GetRequest get_request = {
				.region_id = std::stoi(request.query().get("regionID").value_or("-1")),
				.time_range_start = std::stoll(request.query().get("timeRangeStart").value_or("0")),
				.time_range_end = std::stoll(request.query().get("timeRangeEnd").value_or("0")),
				.number_days = std::stoi(request.query().get("numberDays").value_or("0")),

				.sort_order = request.query().get("sortOrder").value_or("price-asc") == "price-asc"
					? SortOrder::ASCENDING
					: SortOrder::DESCENDING,
				.page = std::stoi(request.query().get("page").value_or("1")),
				.page_size = std::stoi(request.query().get("pageSize").value_or("10")),
				.price_range_width = std::stoi(request.query().get("priceRangeWidth").value_or("1000")),
				.min_free_kilometer_width = std::stoi(request.query().get("minFreeKilometerWidth").value_or("100")),

				.min_number_seats = std::stoi(request.query().get("minNumberSeats").value_or("0")),
				.min_price = std::stoi(request.query().get("minPrice").value_or("0")),
				.max_price = std::stoi(request.query().get("maxPrice").value_or("0")),
				.car_type = car_type_from_string(request.query().get("carType").value_or("all")),
				.only_vollkasko = request.query().get("onlyVollkasko").value_or("false") == "true",
				.min_free_kilometer = std::stoi(request.query().get("minFreeKilometer").value_or("0"))};

			std::cout << "OFFER" << std::endl;

			Offers offers = database.get(get_request);

			std::cout << "Json die nullte" << offers.offers.size() << std::endl;

			// Convert the Offers object to JSON
			json response_json = {{"offers", json::array()},
								  {"price_ranges", json::array()},
								  {"car_type_counts",
								   {{"small_count", offers.car_type_counts.small_count},
									{"sports_count", offers.car_type_counts.sports_count},
									{"luxury_count", offers.car_type_counts.luxury_count},
									{"family_count", offers.car_type_counts.family_count}}},
								  {"seat_counts", json::array()},
								  {"free_kilometer_ranges", json::array()},
								  {"voll_kasko_count",
								   {{"vollkasko_true_count", offers.voll_kasko_count.vollkasko_true_count},
									{"vollkasko_false_count", offers.voll_kasko_count.vollkasko_false_count}}}};

			std::cout << "Json die erste" << std::endl;

			// Serialize the `offers` field
			for (const auto &offer : offers.offers) {
				response_json["offers"].push_back({
					{"id", offer.id},
					{"data", std::string(offer.data.begin(), offer.data.end())} // Convert array<char, 256> to string
				});
			}

			// Serialize the `price_ranges` field
			for (const auto &range : offers.price_ranges) {
				response_json["price_ranges"].push_back({{"price_range_start", range.price_range_start},
														 {"price_range_end", range.price_range_end},
														 {"price_range_count", range.price_range_count}});
			}

			// Serialize the `seat_counts` field
			for (const auto &seat : offers.seat_counts) {
				response_json["seat_counts"].push_back({{"number_seats", seat.number_seats}, {"count", seat.count}});
			}

			// Serialize the `free_kilometer_ranges` field
			for (const auto &range : offers.free_kilometer_ranges) {
				response_json["free_kilometer_ranges"].push_back(
					{{"free_kilometer_range_start", range.free_kilometer_range_start},
					 {"free_kilometer_range_end", range.free_kilometer_range_end},
					 {"free_kilometer_range_count", range.free_kilometer_range_count}});
			}

			std::cout << "json die letzte" << std::endl;


			// Send the JSON response
			response.send(Pistache::Http::Code::Ok, response_json.dump());

			std::cout << "Fertig" << std::endl;
		}
		catch (const std::exception &e) {
			std::cout << "CATCH" << e.what() << std::endl;

			// Handle JSON parsing errors or missing fields
			response.send(Pistache::Http::Code::Bad_Request, std::string("Error parsing request: ") + e.what());
		}
		std::cout << "Fertig x 2" << std::endl;
	}


	// DELETE /api/offers - Clears all offers
	void deleteOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {
		std::cout << "deleteOffers" << std::endl;
		try {
			// database.clear_offers(); // Assuming the database has a method to clear all offers
			std::cout << "All offers cleared from the database" << std::endl;
			response.send(Pistache::Http::Code::Ok, "Data was cleaned up");
		}
		catch (const std::exception &e) {
			response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
		}
	}


private:
	DataBase & database;
};


void setupRoutes(Pistache::Rest::Router &router, OffersHandler &offersHandler) {
	using namespace Pistache::Rest;
	Routes::Post(router, "/api/offers", Routes::bind(&OffersHandler::postOffers, &offersHandler));
	Routes::Delete(router, "/api/offers", Routes::bind(&OffersHandler::deleteOffers, &offersHandler));
	Routes::Get(router, "/api/offers", Routes::bind(&OffersHandler::getOffers, &offersHandler));
}

#include <fstream>

void queryMemory() {
	std::ifstream meminfo("/proc/meminfo");
	std::string line;
	while (std::getline(meminfo, line)) {
		if (line.find("MemAvailable:") == 0) {
			std::cout << "Available Memory: " << line << '\n';
			break;
		}
	}
}


int main() {
	DataBase database;
	OffersHandler offersHandler(database);

	Pistache::Address address("*:80"); // Bind to all network interfaces on port 80
	Pistache::Http::Endpoint server(address);

	Pistache::Rest::Router router;
	setupRoutes(router, offersHandler);

	auto options = Pistache::Http::Endpoint::options().threads(1);
	// .flags(Pistache::Tcp::Options::ReuseAddr).maxRequestSize(1 * 1024 * 1024);  // Allow 100 MB payloads (you can
	// adjust this as needed);

	server.init(options);

	server.setHandler(router.handler());

	std::cout << "Starting server on port 80..." << std::endl;

	server.serve();
	server.shutdown();

	return 0;
}

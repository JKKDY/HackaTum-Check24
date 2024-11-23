# pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace db {

	enum class CarType {
		SMALL = 0x1,
		SPORTS = 0x2,
		LUXURY = 0x4,
		FAMILY = 0x8,
		ALL = LUXURY | FAMILY | SPORTS | SMALL
	};

	struct Offer {
		int64_t start_date;             // Start date in ms since UNIX epoch
		int64_t end_date;               // End date in ms since UNIX epoch
		std::string id;                 // Unique identifier of the offer
		std::array<char, 256> data;     // Base64 encoded 256-byte array
		int region_id;                  // ID of the most specific region (leaf node)
		int number_seats;               // Number of seats in the car
		int price;                      // Price in cents
		int free_kilometers;            // Number of free kilometers included
		CarType car_type;               // Car type the offer belongs to
		bool has_vollkasko;             // Whether the offer has Vollkasko
	};

	enum class SortOrder {
		DESCENDING,
		ASCENDING
	};

	struct GetRequest {
		int region_id;                   // Required: Region ID for which offers are returned
		int64_t time_range_start;        // Required: Timestamp (ms since UNIX epoch) from when offers are considered (inclusive)
		int64_t time_range_end;          // Required: Timestamp (ms since UNIX epoch) until when offers are considered (inclusive)
		int number_days;                 // Required: Number of full days (24h) the car is available within the range_start and range_end

		SortOrder sort_order;            // Required: The order in which offers are returned (price-asc, price-desc)
		int page;                        // Required: The page number from pagination
		int page_size;                   // Required: The number of offers per page
		int price_range_width;           // Required: The width of the price range blocks in cents
		int min_free_kilometer_width;    // Required: The width of the min free kilometer in km

		int min_number_seats = 0;        // Optional: Minimum number of seats the returned cars each have
		double min_price = 0.0;          // Optional: Minimum (inclusive) price the offers have in cents
		double max_price = 0.0;          // Optional: Maximum (exclusive) price the offers have in cents
		CarType car_type = CarType::ALL; // Optional: Car type (small, sports, luxury, family)
		bool only_vollkasko = false;     // Optional: Whether only offers with Vollkasko are returned
		int min_free_kilometer = 0;      // Optional: Minimum number of kilometers included for free
	};


	struct Offers {
		struct Offer {
			std::string id;             //The unique identifier of the offer
			std::string data;           //Base64 encoded 256-byte array representing additional data
		};

		struct PriceRanges {
			int price_range_start;          //The start of the price range in cent
			int price_range_end;            //The end of the price range in cent
			int price_range_count;          //The number of offers in this price range
		};

		struct CarTypeCounts {
			int small_count;                //The number of offers with the car type small
			int sports_count;               //The number of offers with the car type sports
			int luxury_count;               //The number of offers with the car type luxury
			int family_count;               //The number of offers with the car type family
		};

		struct SeatsCount {
			int number_seats;               //The number of seats the cars have
			int seat_count;                 //The number of offers with the given number of seats
		};

		struct FreeKilometerRange {
			int free_kilometer_range_start; //The start of the free kilometer range
			int free_kilometer_range_end;   //The end of the free kilometer range
			int free_kilometer_range_count; //The number of offers in this free kilometer range
		};

		struct VollKaskoCount {
			int vollkasko_true_count;       // Required: The number of offers with Vollkasko
			int vollkasko_false_count;      // Required: The number of offers without Vollkasko
		};

		std::vector<Offer> offers;
		std::vector<PriceRanges> price_ranges;
		CarTypeCounts car_type_counts;
		std::vector<SeatsCount> seat_counts;
		std::vector<FreeKilometerRange> free_kilometer_ranges;
		VollKaskoCount voll_kasko_count;
	};


class DataBase {
	public:
	DataBase();

	void add_offer(const Offer& offer);
	Offers get(const GetRequest& req);

private:
	std::vector<Offer> offers;
};

}



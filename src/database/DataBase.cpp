
#include "DataBase.h"
#include <algorithm>
#include <ranges>

#include <iostream>

namespace db {

	DataBase::DataBase() {}
	void DataBase::add_offer(const Offer &offer) {
		offers.push_back(offer);
		std::cout << "added offer no." << offers.size() << std::endl;
	}

	Offers DataBase::get(const GetRequest &req) {
		Offers ret;

		std::vector<char> vec;
		vec.reserve(1024*1024);
		std::cout << "Davor" << std::endl;

		std::cout << "Size:" << std::endl;
		// for (const auto &offer : offers) {
		// 	std::cout << sizeof(offer) << std::endl;
		// }
		std::cout << offers.size() << std::endl;

		std::cout << "Davor x2" << std::endl;


		std::vector<Offer> valid_offers;
		valid_offers.reserve(offers.size());

		int max_seats = 0;
		int max_free_km = 0;
		int max_price = 0;

		std::cout << "Ok" << std::endl;

		for (auto offer : offers) {
			if (offer.region_id == req.region_id &&
				req.time_range_start <= offer.start_date && offer.end_date <= req.time_range_end &&
				req.number_days >= offer.end_date - offer.start_date &&
				req.min_number_seats <= offer.number_seats &&
				req.min_price <= offer.price && offer.price < req.max_price &&
				req.car_type & offer.car_type &&
				req.only_vollkasko <= offer.has_vollkasko &&
				req.min_free_kilometer <= offer.free_kilometers
				) {
				std::cout << "in the loop" << std::endl;
				valid_offers.push_back(offer);
				if (offer.number_seats > max_seats) {
					max_seats = offer.number_seats;
				}
				if (offer.free_kilometers > max_free_km) {
					max_free_km = offer.free_kilometers;
				}
				if (offer.price > max_price) {
					max_price = offer.price;
				}
			}
		}

		std::cout << "Ok x 2 " << valid_offers.size()  << std::endl;


		int min_idx = req.page * req.page_size;
		int max_idx = (req.page + 1) * req.page_size;

		std::ranges::sort(valid_offers, [&req](const Offer& a, const Offer& b) {
				return req.sort_order == SortOrder::ASCENDING ? a.price < b.price : a.price > b.price;
		});

		auto selected_offers = valid_offers | std::views::drop(min_idx) | std::views::take(max_idx - min_idx);

		// create price bins
		std::cout << req.min_price << "  PRICE   " << req.max_price << std::endl;
		std::cout << req.price_range_width << std::endl;
		for (int i = std::max(req.min_price, 0); i < max_price; i += req.price_range_width) {
			ret.price_ranges.emplace_back(i, i + req.price_range_width);
		}

		// create free Km bins
		for (int i = req.min_free_kilometer; i < max_free_km; i += req.min_free_kilometer) {
			ret.free_kilometer_ranges.emplace_back(i, i + req.min_free_kilometer, 0);
		}

		// create seats bins
		for (int i = req.min_number_seats; i < max_seats; i++) {
			ret.seat_counts.push_back(Offers::SeatsCount{i, 0});
		}

		std::cout << "Ok x 3" << valid_offers.size()  << std::endl;


		for (auto x : selected_offers) {
			// offers
			ret.offers.push_back(Offers::Offer{x.id, x.data});
			std::cout << "loop 1" << std::endl;
			// car type
			switch (x.car_type) {
				case CarType::SMALL: ret.car_type_counts.small_count++; break;
				case CarType::FAMILY: ret.car_type_counts.family_count++; break;
				case CarType::LUXURY: ret.car_type_counts.luxury_count++; break;
				case CarType::SPORTS: ret.car_type_counts.sports_count++; break;
				default: break;
			}
			std::cout << "loop 1" << std::endl;

			// vollkasko
			if (x.has_vollkasko) {
				ret.voll_kasko_count.vollkasko_true_count++;
			}else {
				ret.voll_kasko_count.vollkasko_false_count++;
			}
			std::cout << "loop 2" << std::endl;

			// price ranges
			const int price_idx = (x.price - req.min_price) / req.price_range_width;
			std::cout << "loop 3: idx " << price_idx << " price ranges size: " << ret.price_ranges.size() << std::endl;
			ret.price_ranges[price_idx].price_range_count++;

			//seats count
			std::cout << "loop 4 # seats " << x.number_seats << " size: " << ret.seat_counts.size() <<std::endl;
			ret.seat_counts[x.number_seats - 1].count ++;

			// freeKmrange
			const int free_km_idx = (x.free_kilometers - req.min_free_kilometer) / req.min_free_kilometer_width;
			std::cout << "loop 5 # free km " << free_km_idx << " size: " << ret.free_kilometer_ranges.size() <<std::endl;
			ret.free_kilometer_ranges[free_km_idx].free_kilometer_range_count++;
		}

		std::cout << "Ok x 4 " << valid_offers.size()  << std::endl;

		return ret;
	}
}

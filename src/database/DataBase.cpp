
#include <algorithm>
#include <ranges>
#include "DataBase.h"

namespace db {

	DataBase::DataBase() {}
	void DataBase::add_offer(const Offer &offer) { offers.push_back(offer); }

	Offers DataBase::get(const GetRequest &req) {
		Offers ret;

		std::vector<Offer> valid_offers;

		int max_seats = 0;
		int max_free_km = 0;

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
				valid_offers.push_back(offer);
				if (offer.number_seats > max_seats) {
					max_seats = offer.number_seats;
				}
				if (offer.free_kilometers > max_free_km) {
					max_free_km = offer.free_kilometers;
				}
			}
		}

		int min_idx = req.page * req.page_size;
		int max_idx = (req.page + 1) * req.page_size;

		std::ranges::sort(offers, [&req](const Offer& a, const Offer& b) {
				return req.sort_order == SortOrder::ASCENDING ? a.price < b.price : a.price > b.price;
		});

		auto selected_offers = offers | std::views::drop(min_idx) | std::views::take(max_idx - min_idx);

		// create price bins
		for (int i = req.min_price; i < req.max_price; i += req.price_range_width) {
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

		for (auto x : selected_offers) {
			// offers
			ret.offers.push_back(Offers::Offer{x.id, x.data});

			// car type
			switch (x.car_type) {
			case CarType::SMALL: ret.car_type_counts.small_count++; break;
			case CarType::FAMILY: ret.car_type_counts.family_count++; break;
			case CarType::LUXURY: ret.car_type_counts.luxury_count++; break;
			case CarType::SPORTS: ret.car_type_counts.sports_count++; break;
			default: break;
			}

			// vollkasko
			if (x.has_vollkasko) {
				ret.voll_kasko_count.vollkasko_true_count++;
			}else {
				ret.voll_kasko_count.vollkasko_false_count++;
			}

			// price ranges
			const int price_idx = (x.price - req.min_price) / req.price_range_width;
			ret.price_ranges[price_idx].price_range_count++;

			//seats count
			ret.seat_counts[x.number_seats - 1].count ++;

			// freeKmrange
			const int free_km_idx = (x.free_kilometers - req.min_free_kilometer) / req.min_free_kilometer_width;
			ret.free_kilometer_ranges[free_km_idx].free_kilometer_range_count++;
		}


		return ret;
	}
}

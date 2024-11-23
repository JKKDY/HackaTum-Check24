
#include <algorithm>
#include <ranges>
#include "DataBase.h"

namespace db {

	DataBase::DataBase() {}
	void DataBase::add_offer(const Offer &offer) { offers.push_back(offer); }

	Offers DataBase::get(const GetRequest &req) {
		Offers ret;

		std::vector<Offer> valid_offers;

		for (auto offer : offers) {
			if (offer.region_id == req.region_id &&
				req.time_range_start <= offer.start_date && offer.end_date <= req.time_range_end &&
				req.number_days >= offer.end_date - offer.start_date &&
				req.min_number_seats <= offer.number_seats &&
				req.min_price <= offer.price && offer.price < req.max_price &&
				req.car_type & offer.car_type &&
				req.only_vollkasko <= offer.has_vollkasko &&
				req.min_free_kilometer < offer.free_kilometers
				) {
				valid_offers.push_back(offer);
			}
		}

		int min_idx = req.page * req.page_size;
		int max_idx = (req.page + 1) * req.page_size;

		std::ranges::sort(offers, [&req](const Offer& a, const Offer& b) {
				return req.sort_order == SortOrder::ASCENDING ? a.price < b.price : a.price > b.price;
		});

		auto selected_offers = offers | std::views::drop(min_idx) | std::views::take(max_idx - min_idx);


		for (int i = req.min_price; i < req.max_price; i += req.price_range_width) {
			const int start_price = i;
			const int end_price = std::min(i + req.price_range_width, req.max_price);
			ret.price_ranges.push_back(Offers::PriceRanges{start_price, end_price});
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
			int price_idx = (x.price - req.min_price) / req.price_range_width;
			ret.price_ranges[price_idx].price_range_count++;

			if (x.number_seats > ret.seat_counts.size()) {
				ret.seat_counts.resize(x.number_seats);
			}
			offers
		}

		// freeKmrange
		//seats_count

		return ret;
	}
}

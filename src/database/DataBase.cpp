
#include "DataBase.h"


namespace db {

	DataBase::DataBase() {}
	void DataBase::add_offer(const Offer &offer) {
		offers.push_back(offer);
	}

	Offers DataBase::get(const GetRequest &req) {
		Offers ret;

		std::vector<Offer> valid_offers;

		for (auto offer : offers) {

		}

	}
}

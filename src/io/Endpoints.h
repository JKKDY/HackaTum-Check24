#pragma once
#include <nlohmann/json.hpp>
#include <pistache/http.h>
#include <pistache/router.h>

#include "database/DataBase.h"

namespace db::io {
	class OffersHandler {
	public:
		explicit OffersHandler(db::DataBase & db) : database(db) {}
		void postOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
		void getOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
	private:
		Offer read_offer(const nlohmann::basic_json<> & offerJson);
		DataBase & database;
	};
}
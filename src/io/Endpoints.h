// #pragma once
// #include <pistache/endpoint.h>
// #include <pistache/http.h>
// #include <pistache/router.h>
// #include "src/Database.h"
//
// namespace db::io {
// 	class OffersHandler {
// 	public:
// 		OffersHandler(db::DataBase & db) : database(db) {}
// 		void postOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
// 		void getOffers(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
// 	private:
// 		DataBase & database;
// 	};
// }
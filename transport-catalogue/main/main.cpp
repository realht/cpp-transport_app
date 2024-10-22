#include <iostream>
#include <string>

#include "../json/json_reader.h"
#include "../img/map_renderer.h"
#include "request_handler.h"


using namespace std;

using namespace transportcatalogue;


int main() {

    JSONReader reader(cin);
    const TransportCatalogue catalogue = reader.GetTransportCatalague();

    const RenderSettings rs = reader.GetRenderSettings();
    const RouteSetting rstg = reader.GetRoutSetting();

    const renderer::MapRenderer mr(rs);

    const RequestHandler rq(catalogue, reader.GetRequestList(), mr, rstg);
    rq.AnswerOnRequests();

}
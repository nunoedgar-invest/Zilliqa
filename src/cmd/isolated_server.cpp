/*
 * Copyright (C) 2019 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <boost/program_options.hpp>
#include <iostream>
#include "depends/safeserver/safehttpserver.h"
#include "libServer/IsolatedServer.h"
#include "libServer/LookupServer.h"

#define SUCCESS 0
#define ERROR_IN_COMMAND_LINE -1
#define ERROR_UNHANDLED_EXCEPTION -2

using namespace std;
namespace po = boost::program_options;

int readAccountJsonFromFile(const string& path) {
  ifstream in(path.c_str());

  if (!in) {
    cerr << "Cannot open file \n" << path << endl;
    return -1;
  }
  Json::Value _json;
  in >> _json;

  try {
    for (const auto& i : _json.getMemberNames()) {
      cout << i << endl;
      Address addr(i);
      uint128_t balance(_json[i]["amount"].asString());
      AccountStore::GetInstance().AddAccount(
          addr, {balance, _json[i]["nonce"].asUInt()});
    }
  } catch (exception& e) {
    cout << "Unable to load data " << e.what() << endl;
    return -1;
  }
  return 0;
}

void help(const char* argv[]) {
  cout << "Usage" << endl;
  cout << argv[0] << " --file [Path to Json Account File]" << endl;
}

int main(int argc, const char* argv[]) {
  string accountJsonFilePath;
  uint port = 5555;

  try {
    po::options_description desc("Options");

    desc.add_options()("help,h", "Print help message")(
        "file,f", po::value<string>(&accountJsonFilePath)->required(),
        "Json file containing bootstrap accounts")(
        "port,p", po::value<uint>(&port), "Port to run server on");

    po::variables_map vm;

    try {
      po::store(po::parse_command_line(argc, argv, desc), vm);

      if (vm.count("help")) {
        help(argv);
        cout << desc << endl;
        return SUCCESS;
      }
      po::notify(vm);
    } catch (boost::program_options::required_option& e) {
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      std::cout << desc;
      return ERROR_IN_COMMAND_LINE;
    } catch (boost::program_options::error& e) {
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      return ERROR_IN_COMMAND_LINE;
    }

    INIT_STDOUT_LOGGER();

    LOG_MARKER();

    PairOfKey key;
    Peer peer;

    Mediator mediator(key, peer);
    Node node(mediator, 0, false);
    auto vd = make_shared<Validator>(mediator);

    mediator.RegisterColleagues(nullptr, &node, nullptr, vd.get());

    AccountStore::GetInstance().Init();

    auto isolatedServerConnector = make_unique<jsonrpc::SafeHttpServer>(port);
    auto isolatedServer =
        make_shared<IsolatedServer>(mediator, *isolatedServerConnector);

    if (!isolatedServer
             ->jsonrpc::AbstractServer<IsolatedServer>::StartListening()) {
      cerr << "Server failed to listen" << endl;
      return ERROR_UNHANDLED_EXCEPTION;
    } else {
      cout << "Server listening on " << port << endl;
    }

    while (true) {
      ;  // keep server running
    }

    if (readAccountJsonFromFile(accountJsonFilePath)) {
      cerr << "ERROR: "
           << "Unable to parse account json file" << endl;
      return ERROR_IN_COMMAND_LINE;
    }
  } catch (std::exception& e) {
    std::cerr << "Unhandled Exception reached the top of main: " << e.what()
              << ", application will now exit" << std::endl;
    return ERROR_UNHANDLED_EXCEPTION;
  }

  return SUCCESS;
}
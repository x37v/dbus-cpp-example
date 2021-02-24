/*
 *
 dbus-monitor --session interface=info.x37v.example
 dbus-send --session --type=signal /info/x37v/example info.x37v.example.Example boolean:true
 *
 */

#include <iostream>

#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/signal.h>
#include <core/dbus/object.h>
#include <core/dbus/skeleton.h>
#include <core/dbus/announcer.h>
#include <core/dbus/macros.h>

#include <core/dbus/asio/executor.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>
#include <core/dbus/types/struct.h>


using std::cout;
using std::endl;

namespace {
  class IExampleService {
    public:
      virtual ~IExampleService() = default;

      static core::dbus::types::ObjectPath object_path() {
        static core::dbus::types::ObjectPath p("/info/x37v/example");
        return p;
      }

      struct Signals
      {
        DBUS_CPP_SIGNAL_DEF(Example, IExampleService, bool)
      };
  };

  class ExampleService: public core::dbus::Skeleton<IExampleService> {
    public:
      typedef std::shared_ptr<ExampleService> Ptr;
      ExampleService(const core::dbus::Bus::Ptr& bus) :
        core::dbus::Skeleton<IExampleService>(bus),
        mObject(access_service()->add_object_for_path(IExampleService::object_path())) {
      }
      ~ExampleService() = default;

      void doit() {
        mObject->emit_signal<IExampleService::Signals::Example, bool>(true);
      }
    private:
      core::dbus::Object::Ptr mObject;
  };
}

namespace core
{
  namespace dbus
  {
    namespace traits
    {
      template<>
        struct Service<IExampleService>
        {
          inline static const std::string& interface_name()
          {
            static const std::string s("info.x37v.example");
            return s;
          }
        };
    }
  }
}


int main(int argc, const char * argv[]) {
  std::thread clientThread;

  //service
  auto serviceThread = std::thread([]{
    auto bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    auto ex = core::dbus::asio::make_executor(bus);
    bus->install_executor(ex);
    auto t = std::thread(std::bind(&core::dbus::Bus::run, bus));
    auto service = core::dbus::announce_service_on_bus<IExampleService, ExampleService>(bus);

    while (true) {
      service->doit();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (t.joinable())
      t.join();
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  //client
  {
    auto bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    auto ex = core::dbus::asio::make_executor(bus);
    bus->install_executor(ex);
    clientThread = std::thread(std::bind(&core::dbus::Bus::run, bus));
    auto service = core::dbus::Service::use_service(bus, core::dbus::traits::Service<IExampleService>::interface_name());

    auto obj = service->object_for_path(IExampleService::object_path());
    auto sig = obj->get_signal<IExampleService::Signals::Example>();
    sig->connect([](const bool& arg) {
        cout << "got signal " << arg << endl;
    });

  }

  if (serviceThread.joinable())
    serviceThread.join();
  if (clientThread.joinable())
    clientThread.join();
  return 0;
}

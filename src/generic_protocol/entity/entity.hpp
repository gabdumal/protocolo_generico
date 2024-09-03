#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <uuid.h>

#include <functional>
#include <list>
#include <memory>
#include <pretty_console.hpp>

#include "package.hpp"

using namespace std;

enum class ConnectionStep { SYN, ACK_SYN, ACK_ACK_SYN };

using InternalConnectFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, ConnectionStep>;
using ConnectFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, uuids::uuid, ConnectionStep>;
using ConnectFunction = shared_ptr<
    function<void(ConnectFunctionParameters connect_function_parameters)>>;

using InternalRemoveConnectionFunctionParameters = tuple<uuids::uuid>;
using RemoveConnectionFunctionParameters = tuple<uuids::uuid, uuids::uuid>;
using RemoveConnectionFunction = shared_ptr<function<void(
    RemoveConnectionFunctionParameters remove_connection_function_parameters)>>;

using InternalIsConnectedAtStepFunctionParameters =
    tuple<uuids::uuid, ConnectionStep>;
using IsConnectedAtStepFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, ConnectionStep>;
using IsConnectedAtStepFunction =
    shared_ptr<function<bool(IsConnectedAtStepFunctionParameters
                                 is_connected_at_step_function_parameters)>>;

using InternalCanSendPackageFunctionParameters = tuple<uuids::uuid>;
using CanSendPackageFunctionParameters = tuple<uuids::uuid, uuids::uuid>;
using CanSendPackageFunction = shared_ptr<function<bool(
    CanSendPackageFunctionParameters can_send_package_function_parameters)>>;

using InternalCanStoreDataFunctionParameters = tuple<uuids::uuid, uuids::uuid>;
using CanStoreDataFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, uuids::uuid>;
using CanStoreDataFunction = shared_ptr<function<bool(
    CanStoreDataFunctionParameters can_store_data_function_parameters)>>;

using EnqueuePackageFunctionParameters = tuple<uuids::uuid, uuids::uuid>;
using EnqueuePackageFunction = shared_ptr<function<void(
    EnqueuePackageFunctionParameters enqueue_package_function_parameters)>>;

using InternalDequeuePackageFunctionParameters =
    tuple<uuids::uuid, uuids::uuid>;
using DequeuePackageFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, uuids::uuid>;
using DequeuePackageFunction = shared_ptr<function<void(
    DequeuePackageFunctionParameters dequeue_package_function_parameters)>>;

class Entity {
   private:
    uuids::uuid id;
    string name;
    string storage;

    ConnectFunction connect_function;
    RemoveConnectionFunction remove_connection_function;
    IsConnectedAtStepFunction is_connected_at_step_function;
    CanSendPackageFunction can_send_package_function;
    CanStoreDataFunction can_store_data_function;
    EnqueuePackageFunction enqueue_package_function;
    DequeuePackageFunction dequeue_package_function;

    /* Methods */

    void printInformation(
        string information, ostream &output_stream,
        PrettyConsole::Color color = PrettyConsole::Color::DEFAULT) const;

    optional<Package> receiveSynPackage(
        const Package &package,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Package> receiveFinPackage(
        const Package &package,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Package> receiveAckPackage(
        const Package &package,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Package> receiveAckSynPackage(
        const Package &package, uuids::uuid sent_message_id,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Package> receiveAckAckSynPackage(
        const Package &package, uuids::uuid sent_message_id,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Package> receiveNackPackage(
        const Package &package,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Package> receiveDataPackage(
        const Package &package,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);

   public:
    /* Construction */
    Entity(uuids::uuid id, string name, ConnectFunction connect_function,
           RemoveConnectionFunction remove_connection_function,
           IsConnectedAtStepFunction is_connected_at_step_function,
           CanSendPackageFunction can_send_package_function,
           CanStoreDataFunction can_store_data_function,
           DequeuePackageFunction dequeue_package_function)
        : id(id),
          name(name),
          storage(""),
          connect_function(connect_function),
          remove_connection_function(remove_connection_function),
          is_connected_at_step_function(is_connected_at_step_function),
          can_send_package_function(can_send_package_function),
          can_store_data_function(can_store_data_function),
          dequeue_package_function(dequeue_package_function) {}

    ~Entity() {}

    /* Getters */
    uuids::uuid getId() const;
    string getName() const;

    /* Setters */
    void setName(string name);

    /* Methods */
    bool canSendMessage(uuids::uuid message_id) const;

    bool sendMessage(Message message, bool should_be_confirmed);
    optional<Package> receivePackage(
        Package package,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);

    void printPackageInformation(Package package, ostream &output_stream,
                                 bool is_sending) const;
    void printStorage(function<void(string)> print_message) const;

    /* Connection */
    void connect(InternalConnectFunctionParameters connect_function_parameters);
    void removeConnection(InternalRemoveConnectionFunctionParameters
                              remove_connection_function_parameters);
    bool isConnectedAtStep(InternalIsConnectedAtStepFunctionParameters
                               is_connected_at_step_function_parameters) const;
    bool canSendPackage(InternalCanSendPackageFunctionParameters
                            can_send_package_function_parameters) const;
    bool canStoreData(InternalCanStoreDataFunctionParameters
                          can_store_data_function_parameters) const;
    void dequeuePackage(InternalDequeuePackageFunctionParameters
                            dequeue_package_function_parameters);
};

using EntitiesList = list<shared_ptr<Entity>>;

#endif  // _ENTITY_HPP

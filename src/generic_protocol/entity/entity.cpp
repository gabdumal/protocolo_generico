#include "entity.hpp"

#include "generic_protocol_constants.hpp"
#include "message.hpp"
#include "package.hpp"
#include "util.hpp"

using namespace std;

/* Getters */

uuids::uuid Entity::getId() const { return this->id; }

string Entity::getName() const { return this->name; }

/* Setters */

void Entity::setName(string name) { this->name = name; }

/* Connection */

void Entity::connect(
    InternalConnectFunctionParameters connect_function_parameters) {
    ConnectFunctionParameters parameters = {
        this->id, get<0>(connect_function_parameters),
        get<1>(connect_function_parameters),
        get<2>(connect_function_parameters)};
    this->connect_function->operator()(parameters);
}

void Entity::removeConnection(InternalRemoveConnectionFunctionParameters
                                  remove_connection_function_parameters) {
    RemoveConnectionFunctionParameters parameters = {
        this->id, get<0>(remove_connection_function_parameters)};
    this->remove_connection_function->operator()(parameters);
}

bool Entity::isConnectedAtStep(
    InternalIsConnectedAtStepFunctionParameters
        is_connected_at_step_function_parameters) const {
    IsConnectedAtStepFunctionParameters parameters = {
        this->id, get<0>(is_connected_at_step_function_parameters),
        get<1>(is_connected_at_step_function_parameters)};
    auto is_connected_at_step =
        this->is_connected_at_step_function->operator()(parameters);
    return is_connected_at_step;
}

bool Entity::canSendPackage(InternalCanSendPackageFunctionParameters
                                can_send_package_function_parameters) const {
    CanSendPackageFunctionParameters parameters = {
        this->id, get<0>(can_send_package_function_parameters)};
    auto can_send_package =
        this->can_send_package_function->operator()(parameters);
    return can_send_package;
}

bool Entity::canStoreData(InternalCanStoreDataFunctionParameters
                              can_store_data_function_parameters) const {
    CanStoreDataFunctionParameters parameters = {
        this->id, get<0>(can_store_data_function_parameters),
        get<1>(can_store_data_function_parameters)};
    auto can_store_data = this->can_store_data_function->operator()(parameters);
    return can_store_data;
}

void Entity::dequeuePackage(InternalDequeuePackageFunctionParameters
                                dequeue_package_function_parameters) {
    DequeuePackageFunctionParameters parameters = {
        this->id, get<0>(dequeue_package_function_parameters),
        get<1>(dequeue_package_function_parameters)};
    this->dequeue_package_function->operator()(parameters);
}

/* Methods */

bool Entity::sendMessage(Message message, bool should_be_confirmed) {
    // if (message.getCode() == Message::Code::DATA) {
    //     if (!this->canStoreData({message.getTargetEntityId(),
    //     message.getId()}))
    //         return false;
    // }
    return true;
}

void Entity::printInformation(string information, ostream &output_stream,
                              PrettyConsole::Color color) const {
    string header =
        "Entity " + this->getName() + " [" + to_string(this->getId()) + "]";
    PrettyConsole::Decoration header_decoration(PrettyConsole::Color::WHITE,
                                                PrettyConsole::Color::CYAN,
                                                PrettyConsole::Format::BOLD);
    PrettyConsole::Decoration information_decoration(color);
    Util::printInformation(header, information, output_stream,
                           header_decoration, information_decoration);
}

void Entity::printStorage(function<void(string)> print_message) const {
    print_message("=== BEGIN ===");
    istringstream content_stream(this->storage);
    string line;
    while (getline(content_stream, line)) {
        print_message(line);
    }
    print_message("==== END ====");
}

void Entity::printPackageInformation(Package package, ostream &output_stream,
                                     bool is_sending) const {
    if (GenericProtocolConstants::debug_information) {
        ostringstream package_content;
        package.print([&package_content](string line) {
            package_content << PrettyConsole::tab << line << endl;
        });

        string operation =
            is_sending ? "Trying to send package\n" : "Received package\n";
        string information = operation + package_content.str();

        this->printInformation(information, cout,
                               is_sending ? PrettyConsole::Color::YELLOW
                                          : PrettyConsole::Color::MAGENTA);
    }
}

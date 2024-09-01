#ifndef _NETWORK_HPP
#define _NETWORK_HPP

#include "../entity/entity.hpp"
#include <condition_variable>
#include <memory>
#include <message.hpp>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <uuid.h>

using namespace std;

class Network {
private:
  shared_ptr<uuids::uuid_random_generator> uuid_generator;
  string name;
  thread network_thread;

  unordered_map<uuids::uuid, shared_ptr<Entity>> entities;
  mutex entities_mutex;

  queue<Message> messages;
  mutex messages_mutex;

  condition_variable message_processed_cv;
  int processing_messages_count;
  bool can_stop_thread;

  /* Construction */
  Network(string name, shared_ptr<uuids::uuid_random_generator> uuid_generator);

  /* Methods */
  void processMessage(Message message);
  void sendMessage(Message message);
  void printInformation(
      string information, ostream &output_stream,
      PrettyConsole::Color color = PrettyConsole::Color::DEFAULT) const;
  void joinThread();

public:
  /* Destruction */
  ~Network();

  /* Getters */
  string getName() const;

  /* Methods */
  void connectEntity(shared_ptr<Entity> entity);
  void disconnectEntity(uuids::uuid entity_id);
  bool receiveMessage(Message message);

  /* Static Methods */
  static unique_ptr<Network>
  createNetwork(string name,
                shared_ptr<uuids::uuid_random_generator> uuid_generator);
};

#endif // _NETWORK_HPP

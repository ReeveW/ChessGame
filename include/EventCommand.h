#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <memory>
#include <vector>

#include "Board.h"

struct UIInfo {
  sf::RenderWindow& window;
  Board& board;
};

class IEventCommand {
 public:
  virtual ~IEventCommand() = default;
  virtual void execute(const sf::Event& event, UIInfo& context) = 0;
  virtual bool canExecute(const sf::Event& event, UIInfo& context) = 0;
};

class CloseWindowCommand : public IEventCommand {
 public:
  void execute(const sf::Event& event, UIInfo& context);
  bool canExecute(const sf::Event& event, UIInfo& context);
};

class PromoteCommand : public IEventCommand {
 public:
  void execute(const sf::Event& event, UIInfo& context);
  bool canExecute(const sf::Event& event, UIInfo& context);
};

class DragAndDropCommand : public IEventCommand {
 public:
  void execute(const sf::Event& event, UIInfo& context);
  bool canExecute(const sf::Event& event, UIInfo& context);
};

class CommandDispatcher {
 private:
  std::vector<std::unique_ptr<IEventCommand>> commands;

 public:
  void add(std::unique_ptr<IEventCommand> command);
  void dispatch(const sf::Event& event, UIInfo& context);
};

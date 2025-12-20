#include "EventCommand.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <vector>

// Need to implement UIInfo into all commands, but I first need to create data
// structures that store all relevant information that the commands need.
// this includes: a way to know if a pawn is currently promoting, and know all
// the info that dragAndDrop, findPiece, and dropPiece know. I want to cover
// this with the struct UIInfo.

void CloseWindowCommand::execute(const sf::Event& event, UIInfo& context) {
  // close window
  // STUB
  return;
}

bool CloseWindowCommand::canExecute(const sf::Event& event, UIInfo& context) {
  return event.type == sf::Event::Closed;
}

void PromoteCommand::execute(const sf::Event& event, UIInfo& context) {
  // STUB
  return;
}

bool PromoteCommand::canExecute(const sf::Event& event, UIInfo& context) {
  // STUB
  return false;
}

void DragAndDropCommand::execute(const sf::Event& event, UIInfo& context) {
  // STUB
  return;
}

bool DragAndDropCommand::canExecute(const sf::Event& event, UIInfo& context) {
  return event.type == sf::Event::MouseButtonPressed &&
         event.mouseButton.button == sf::Mouse::Left;
}

void CommandDispatcher::add(std::unique_ptr<IEventCommand> command) {
  // STUB
  this->commands.push_back(std::move(command));
  return;
}

void CommandDispatcher::dispatch(const sf::Event& event, UIInfo& context) {
  // STUB
  for (const auto& command : this->commands) {
    if (command->canExecute(event, context)) {
      command->execute(event, context);
      return;
    }
  }
}

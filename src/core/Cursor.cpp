#include "../include/core/Cursor.h"
#include <algorithm>
#include <cstdint>

using namespace mexedit::core;

Cursor::Cursor(PositionValidator validator) : validator_(std::move(validator))
{
    position_ = {0, 0};
}

void Cursor::setPosition(const Position& pos)
{
    updatePosition(pos);
}

void Cursor::moveUp(size_t lines)
{
    Position newPos = position_;
    if (newPos.line >= lines)
    {
        newPos.line -= lines;
    }
    else
    {
        newPos.line = 0;
    }
    updatePosition(newPos);
}

void Cursor::moveDown(size_t lines)
{
    Position newPos = position_;
    newPos.line += lines;
    updatePosition(newPos);
}

void Cursor::moveLeft(size_t columns)
{
    Position newPos = position_;
    if (newPos.column >= columns)
    {
        newPos.column -= columns;
    }
    else
    {
        if (newPos.line > 0)
        {
            newPos.line--;
            newPos.column = SIZE_MAX;
        }
        else
        {
            newPos.column = 0;
        }
    }
    updatePosition(newPos);
}

void Cursor::moveRight(size_t columns)
{
    Position newPos = position_;
    newPos.column += columns;
    updatePosition(newPos);
}

void Cursor::moveToLineStart()
{
    Position newPos = position_;
    newPos.column = 0;
    updatePosition(newPos);
}

void Cursor::moveToLineEnd()
{
    Position newPos = position_;
    newPos.column = SIZE_MAX;
    updatePosition(newPos);
}

void Cursor::moveToDocumentStart()
{
    updatePosition({0, 0});
}

void Cursor::moveToDocumentEnd()
{
    Position newPos = {SIZE_MAX, SIZE_MAX};
    updatePosition(newPos);
}

void Cursor::updatePosition(const Position& newPos)
{
    Position validatedPos = newPos;
    
    if (validator_)
    {
        validatedPos = validator_(newPos);
    }
    
    if (position_ != validatedPos)
    {
        position_ = validatedPos;
        
        if (moveCallback_)
        {
            moveCallback_(position_);
        }
    }
}
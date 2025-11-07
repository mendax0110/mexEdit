#include "core/Cursor.h"
#include "utils/MemoryDebugger.h"
#include "utils/Logger.h"
#include <algorithm>
#include <cstdint>

using namespace mexedit::core;

Cursor::Cursor(PositionValidator validator) : validator_(std::move(validator))
{
    TRACE_FUNC
    TRACK_MEMORY(Cursor, this);
    position_ = {0, 0};
}

Cursor::~Cursor()
{
    TRACE_FUNC
    UNTRACK_MEMORY(Cursor, this);
}

void Cursor::setPosition(const Position& pos)
{
    TRACE_FUNC
    updatePosition(pos);
}

void Cursor::moveUp(size_t lines)
{
    TRACE_FUNC
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
    TRACE_FUNC
    Position newPos = position_;
    newPos.line += lines;
    updatePosition(newPos);
}

void Cursor::moveLeft(size_t columns)
{
    TRACE_FUNC
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
    TRACE_FUNC
    Position newPos = position_;
    newPos.column += columns;
    updatePosition(newPos);
}

void Cursor::moveToLineStart()
{
    TRACE_FUNC
    Position newPos = position_;
    newPos.column = 0;
    updatePosition(newPos);
}

void Cursor::moveToLineEnd()
{
    TRACE_FUNC
    Position newPos = position_;
    newPos.column = SIZE_MAX;
    updatePosition(newPos);
}

void Cursor::moveToDocumentStart()
{
    TRACE_FUNC
    updatePosition({0, 0});
}

void Cursor::moveToDocumentEnd()
{
    TRACE_FUNC
    Position newPos = {SIZE_MAX, SIZE_MAX};
    updatePosition(newPos);
}

void Cursor::updatePosition(const Position& newPos)
{
    TRACE_FUNC
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
// AsioIOServicePool.cpp - Boost.Asio io_context 线程池实现
#include "AsioIOServicePool.h"
#include <iostream>

AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : _io_services(size), _works(size), _next_io_service(0)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        _works[i] = std::unique_ptr<Work>(new Work(_io_services[i]));
    }
    for (std::size_t i = 0; i < _io_services.size(); ++i)
    {
        _threads.emplace_back([this, i]() {
            _io_services[i].run();
        });
    }
}

AsioIOServicePool::~AsioIOServicePool()
{
    stop();
}

boost::asio::io_context &AsioIOServicePool::getIoService()
{
    auto &service = _io_services[_next_io_service++];
    if (_next_io_service == _io_services.size())
    {
        _next_io_service = 0;
    }
    return service;
}

void AsioIOServicePool::stop()
{
    for (auto &work : _works)
    {
        if (work)
        {
            work->get_io_context().stop();
            work.reset();
        }
    }
    _works.clear();
    for (auto &t : _threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    _threads.clear();
}

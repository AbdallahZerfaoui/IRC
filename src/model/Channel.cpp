/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkeil <tkeil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 17:50:46 by tkeil             #+#    #+#             */
/*   Updated: 2025/08/13 15:21:04 by tkeil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"

Channel::Channel(const std::string &name, std::unordered_map<int, Client> &clients) : _name(name), _key(""), _topic(""), _is_private(false), _invite_only(false), _invited_clients(), _members(), _operators(), _clients_ref(clients), _limit(-1)
{
}

Channel::Channel(Channel &&other) : _name(std::move(other._name)), _key(std::move(other._key)), _topic(std::move(other._topic)), _is_private(other._is_private), _invite_only(other._invite_only), _invited_clients(std::move(other._invited_clients)), _members(std::move(other._members)), _operators(std::move(other._operators)), _clients_ref(other._clients_ref), _limit(other._limit) {}

void Channel::add_client(int client_fd)
{
    if (_members.find(client_fd) == _members.end())
    {
        _members.insert(client_fd);
        std::cout << "Client FD " << client_fd << " added to channel " << _name << std::endl;
    }
    else
    {
        std::cerr << "Client FD " << client_fd << " is already in channel " << _name << std::endl;
    }
}

size_t Channel::remove_client(int client_fd)
{
    if (_members.find(client_fd) != _members.end())
    {
        if (!_members.erase(client_fd))
            return (0);
        std::cout << "Client FD " << client_fd << " removed from channel " << _name << std::endl;
    }
    else
    {
        std::cerr << "Client FD " << client_fd << " not found in channel " << _name << std::endl;
        return (0);
    }
    return (1);
}

void Channel::add_operator(int client_fd)
{
    if (_operators.find(client_fd) == _operators.end())
    {
        _operators.insert(client_fd);
        std::cout << "Client FD " << client_fd << " added to the operator list of the channel " << _name << std::endl;
    }
    else
    {
        std::cerr << "Client FD " << client_fd << " is already an operator of the channel " << _name << std::endl;
    }
}

size_t Channel::remove_operator(int client_fd)
{
    if (_operators.find(client_fd) != _operators.end())
    {
        if (!_operators.erase(client_fd))
            return (0);
        std::cout << "Client FD " << client_fd << " was removed from the operator list of the channel " << _name << std::endl;
    }
    else
    {
        std::cerr << "Client FD " << client_fd << " is not an operator of the channel " << _name << std::endl;
        return (0);
    }
    return (1);
}

void Channel::add_invited_client(int client_fd)
{
    if (_invited_clients.find(client_fd) == _invited_clients.end())
    {
        _invited_clients.insert(client_fd);
        std::cout << "Client FD " << client_fd << " added to the invited list of the channel " << _name << std::endl;
    }
    else
    {
        std::cerr << "Client FD " << client_fd << " is already invited to the channel " << _name << std::endl;
    }
}

std::set<int> Channel::get_members() const
{
    return _members;
}

// void Server::send_reply(int fd, int code, const std::vector<std::string>& params, const std::string& msg)
// {
//     std::ostringstream out;
//     out << ':' << _hostname << ' ' << code << ' ';
//     for (size_t i = 0; i < params.size(); ++i)
// 	{
//         if (i)
// 			out << ' ';
//         out << params[i];
//     }
//     out << " :" << msg << "\r\n";
//     ssize_t bytes_sent = send(fd, out.str().c_str(), out.str().size(), 0);
// 	if (bytes_sent == -1)
//     {
//         std::cerr << "send() failed for client FD " << fd << ": " << std::strerror(errno) << std::endl;
//         throw std::runtime_error("send() failed");
//     }
//     if (static_cast<size_t>(bytes_sent) < msg.size())
//     {
//         std::cerr << "Warning: Partial send() for client FD " << fd << std::endl;
//         throw std::runtime_error("Partial send(), incomplete message sent");
//     }
// }

void Channel::broadcast_message(const std::string &message, int sender_fd) const
{
    for (const auto &member_fd : _members)
    {
        if (member_fd != sender_fd)
        {
            try
            {
                std::string msg = message;
                _clients_ref.at(member_fd).send(msg);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to broadcast a message to client: " << e.what() << std::endl;
                return;
            }
        }
    }
}

bool Channel::has_member(int client_fd) const
{
    return _members.find(client_fd) != _members.end();
}

bool Channel::is_invited(int client_fd) const
{
    return _invited_clients.find(client_fd) != _invited_clients.end();
}

bool Channel::is_operator(int client_fd) const
{
    return _operators.find(client_fd) != _operators.end();
}

bool Channel::requires_key() const
{
    return _is_private;
}

void Channel::set_key(const std::string &key)
{
    _key = key;
    _is_private = true;
    std::cout << "Channel " << _name << " is now private with key: " << _key << std::endl;
}

void Channel::remove_key()
{
    _key.clear();
    _is_private = false;
    std::cout << "Channel " << _name << " is now public, key removed." << std::endl;
}

const std::string &Channel::get_channel_key() const { return _key; }

std::string Channel::get_name() const
{
    return _name;
}

void Channel::set_invite_only(bool invite_only)
{
    _invite_only = invite_only;
}
bool Channel::is_invite_only() const
{
    return _invite_only;
}

void Channel::set_topic_protected(bool topic_protected)
{
    _topic_protected = topic_protected;
}

bool Channel::is_topic_protected() const
{
    return _topic_protected;
}

void Channel::set_topic(const std::string &topic)
{
    _topic = topic;
}

const std::string &Channel::get_topic() const
{
    return _topic;
}

void Channel::set_limit(int limit)
{
    _limit = limit;
}

void Channel::remove_limit()
{
    _limit = -1;
}

int Channel::get_limit() const
{
    return _limit;
}

std::string Channel::topic() const
{
	return _topic;
}
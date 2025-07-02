/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkeil <tkeil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 17:50:46 by tkeil             #+#    #+#             */
/*   Updated: 2025/07/02 18:53:36 by tkeil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Channel.hpp"

Channel::Channel(const std::string& name, std::unordered_map<int, Client>& clients) : _name(name), _members(), _clients_ref(clients)
{	
}

Channel::Channel(Channel&& other) : _name(std::move(other._name)), _members(std::move(other._members)), _clients_ref(other._clients_ref) {}

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

std::set<int> Channel::get_members() const
{
	return _members;
}

void Channel::broadcast_message(const std::string& message, int sender_fd) const
{
	for (const auto& member_fd : _members)
	{
		if (member_fd != sender_fd)
		{
			try
			{
				// [#channel1] <bob>: hi everyone => bob from channel1 sends a message to the channel
				std::string msg = "[#" + _name + "] <" + _clients_ref.at(sender_fd).get_nickname() + ">: " + message;
			    _clients_ref.at(member_fd).send(msg);
			}
			catch (const std::exception& e)
			{
			    std::cerr << "Failed to broadcast a message to client: " << e.what() << std::endl;
				return ;
			}
		}
	}
}

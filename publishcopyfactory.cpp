/*
This file is part of FlashMQ (https://www.flashmq.org)
Copyright (C) 2021-2023 Wiebe Cazemier

FlashMQ is free software: you can redistribute it and/or modify
it under the terms of The Open Software License 3.0 (OSL-3.0).

See LICENSE for license details.
*/

#include <cassert>
#include <stdexcept>

#include "publishcopyfactory.h"
#include "mqttpacket.h"

PublishCopyFactory::PublishCopyFactory(MqttPacket *packet) :
    packet(packet),
    orgQos(packet->getQos()),
    orgRetain(packet->getRetain())
{

}

PublishCopyFactory::PublishCopyFactory(Publish *publish) :
    publish(publish),
    orgQos(publish->qos),
    orgRetain(publish->retain)
{

}

MqttPacket *PublishCopyFactory::getOptimumPacket(const uint8_t max_qos, const ProtocolVersion protocolVersion, uint16_t topic_alias, bool skip_topic)
{
    const uint8_t actualQos = getEffectiveQos(max_qos);

    if (packet)
    {
        // The incoming topic alias is not relevant after initial conversion and it should not propagate.
        assert(packet->getPublishData().topicAlias == 0);

        // When the packet contains an alias specific to the receiver, we don't cache it.
        if (protocolVersion >= ProtocolVersion::Mqtt5 && topic_alias > 0)
        {
            this->oneShotPacket.emplace(protocolVersion, packet->getPublishData(), actualQos, topic_alias, skip_topic);
            return &*this->oneShotPacket;
        }

        if (!packet->biteArrayCannotBeReused() &&
            getPublishLayoutCompareKey(packet->getProtocolVersion()) == getPublishLayoutCompareKey(protocolVersion) &&
            static_cast<bool>(orgQos) == static_cast<bool>(actualQos))
        {
            return packet;
        }

        // Note that this cache also possibly contains the expiration interval, but because we're only hitting this block for on-line
        // publishers, the interval has not decreased and is fine.
        const int cache_key = (static_cast<uint8_t>(protocolVersion) * 10) + static_cast<bool>(actualQos);
        std::optional<MqttPacket> &cachedPack = constructedPacketCache[cache_key];

        if (!cachedPack)
        {
            cachedPack.emplace(protocolVersion, packet->getPublishData(), actualQos, 0, false);
        }

        return &*cachedPack;
    }

    // Getting an instance of a Publish object happens at least on retained messages, will messages and SYS topics. It's low traffic, anyway.
    assert(publish);

    // The incoming topic alias is not relevant after initial conversion and it should not propagate.
    assert(publish->topicAlias == 0);

    this->oneShotPacket.emplace(protocolVersion, *publish, actualQos, topic_alias, skip_topic);
    return &*this->oneShotPacket;
}

uint8_t PublishCopyFactory::getEffectiveQos(uint8_t max_qos) const
{
    const uint8_t effectiveQos = std::min<uint8_t>(orgQos, max_qos);
    return effectiveQos;
}

bool PublishCopyFactory::getEffectiveRetain(bool retainAsPublished) const
{
    return orgRetain && retainAsPublished;
}

const std::string &PublishCopyFactory::getTopic() const
{
    if (packet)
        return packet->getTopic();
    assert(publish);
    return publish->topic;
}

const std::vector<std::string> &PublishCopyFactory::getSubtopics()
{
    if (packet)
    {
        return packet->getSubtopics();
    }
    else if (publish)
    {
        return publish->getSubtopics();
    }

    throw std::runtime_error("Bug in &PublishCopyFactory::getSubtopics()");
}

std::string_view PublishCopyFactory::getPayload() const
{
    if (packet)
        return packet->getPayloadView();
    assert(publish);
    return publish->payload;
}

bool PublishCopyFactory::getRetain() const
{
    // Keeping this here as reminder that it should not be implemented.
    assert(false);
    return false;
}

/**
 * @brief PublishCopyFactory::getNewPublish gets a new publish object from an existing packet or publish.
 * @param new_max_qos
 * @return
 *
 * It being a public function, the idea is that it's only needed for creating publish objects for storing QoS messages for off-line
 * clients. For on-line clients, you're always making a packet (with getOptimumPacket()).
 */
Publish PublishCopyFactory::getNewPublish(uint8_t new_max_qos, bool retainAsPublished) const
{
    // (At time of writing) we only need to construct new publishes for QoS (because we're storing QoS publishes for offline clients). If
    // you're doing it elsewhere, it's a bug.
    assert(orgQos > 0);
    assert(new_max_qos > 0);

    const uint8_t actualQos = getEffectiveQos(new_max_qos);

    if (packet)
    {
        assert(packet->getQos() > 0);

        Publish p(packet->getPublishData());
        p.qos = actualQos;
        p.retain = getEffectiveRetain(retainAsPublished);
        return p;
    }

    assert(publish->qos > 0); // Same check as above, but then for Publish objects.

    Publish p(*publish);
    p.qos = actualQos;
    p.retain = getEffectiveRetain(retainAsPublished);
    return p;
}

std::shared_ptr<Client> PublishCopyFactory::getSender()
{
    if (packet)
        return packet->getSender();
    return std::shared_ptr<Client>(0);
}

const std::vector<std::pair<std::string, std::string> > *PublishCopyFactory::getUserProperties() const
{
    if (packet)
    {
        return packet->getUserProperties();
    }

    assert(publish);

    return publish->getUserProperties();
}

const std::optional<std::string> &PublishCopyFactory::getCorrelationData() const
{
    if (packet)
        return packet->getCorrelationData();

    assert(publish);
    return publish->correlationData;
}

const std::optional<std::string> &PublishCopyFactory::getResponseTopic() const
{
    if (packet)
        return packet->getResponseTopic();

    assert(publish);
    return publish->responseTopic;
}

void PublishCopyFactory::setSharedSubscriptionHashKey(size_t hash)
{
    this->sharedSubscriptionHashKey = hash;
}

int PublishCopyFactory::getPublishLayoutCompareKey(ProtocolVersion pv)
{
    switch (pv)
    {
    case ProtocolVersion::None:
        return 0;
    case ProtocolVersion::Mqtt31:
    case ProtocolVersion::Mqtt311:
        return 1;
    case ProtocolVersion::Mqtt5:
        return 2;
    default:
        return 3;
    }
}




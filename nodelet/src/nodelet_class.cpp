/*
 * Copyright (c) 2010, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <nodelet/nodelet.h>
#include <nodelet/detail/callback_queue.h>
#include <nodelet/detail/callback_queue_manager.h>

#include <ros/ros.h>
#include <ros/callback_queue.h>

namespace nodelet
{

Nodelet::Nodelet ()
: inited_(false)
, nodelet_name_("uninitialized")
{
}

Nodelet::~Nodelet ()
{
  bond_.reset();

  if (inited_)
  {
    callback_manager_->removeQueue(st_callback_queue_);
    callback_manager_->removeQueue(mt_callback_queue_);
  }
}

ros::CallbackQueueInterface& Nodelet::getSTCallbackQueue () const
{
  if (!inited_)
  {
    throw UninitializedException("getSTCallbackQueue");
  }

  return *st_callback_queue_;
}

ros::CallbackQueueInterface& Nodelet::getMTCallbackQueue () const
{
  if (!inited_)
  {
    throw UninitializedException("getMTCallbackQueue");
  }

  return *mt_callback_queue_;
}

ros::NodeHandle& Nodelet::getNodeHandle() const
{
  if (!inited_)
  {
    throw UninitializedException("getNodeHandle");
  }

  return *nh_;
}
ros::NodeHandle& Nodelet::getPrivateNodeHandle() const
{
  if (!inited_)
  {
    throw UninitializedException("getPrivateNodeHandle");
  }

  return *private_nh_;
}
ros::NodeHandle& Nodelet::getMTNodeHandle() const
{
  if (!inited_)
  {
    throw UninitializedException("getMTNodeHandle");
  }

  return *mt_nh_;
}
ros::NodeHandle& Nodelet::getMTPrivateNodeHandle() const
{
  if (!inited_)
  {
    throw UninitializedException("getMTPrivateNodeHandle");
  }

  return *mt_private_nh_;
}

void Nodelet::init(const std::string& name, const M_string& remapping_args, const V_string& my_argv, detail::CallbackQueueManager* callback_manager, boost::shared_ptr<bond::Bond> bond)
{
  if (inited_)
  {
    throw MultipleInitializationException();
  }

  bond_ = bond;

  callback_manager_ = callback_manager;
  st_callback_queue_.reset(new detail::CallbackQueue(callback_manager));
  mt_callback_queue_.reset(new detail::CallbackQueue(callback_manager));

  callback_manager->addQueue(st_callback_queue_, false);
  callback_manager->addQueue(mt_callback_queue_, true);

  nodelet_name_ = name;
  my_argv_ = my_argv;

  private_nh_.reset(new ros::NodeHandle(name, remapping_args));
  private_nh_->setCallbackQueue(st_callback_queue_.get());
  nh_.reset(new ros::NodeHandle(ros::names::parentNamespace(name), remapping_args));
  nh_->setCallbackQueue(st_callback_queue_.get());

  mt_nh_.reset(new ros::NodeHandle(ros::names::parentNamespace(name), remapping_args));
  mt_nh_->setCallbackQueue(mt_callback_queue_.get());
  mt_private_nh_.reset(new ros::NodeHandle (name, remapping_args));
  mt_private_nh_->setCallbackQueue(mt_callback_queue_.get());

  NODELET_DEBUG ("Nodelet initializing");
  inited_ = true;
  this->onInit ();
}

} // namespace nodelet

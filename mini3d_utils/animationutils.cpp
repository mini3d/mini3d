
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#include "animationutils.hpp"

void mini3d_assert(bool expression, const char* text, ...) ;

using namespace mini3d::utils;

const unsigned int POSITION_KEYFRAME_SIZE = sizeof(Vec3) + sizeof(float);
const unsigned int ROTATION_KEYFRAME_SIZE = sizeof(Quat) + sizeof(float);

Animation* AnimationUtils::AnimationFromAction(Action* action, Transform* target)
{
    ITrack** tracks = new ITrack*[action->channels.count];

    for (unsigned int i = 0; i < action->channels.count; ++i)
    {
        Channel* channel = action->channels.array + i;
        switch (channel->type) 
        {
            case Channel::POSITION:
                tracks[i] = new Track<Vec3>(&target->pos, (Keyframe<Vec3>*)channel->animationData.array, channel->animationData.count / POSITION_KEYFRAME_SIZE);
                break;
            case Channel::ROTATION:
                tracks[i] = new Track<Quat, true>(&target->rot, (Keyframe<Quat>*)channel->animationData.array, channel->animationData.count / ROTATION_KEYFRAME_SIZE);
                break;
        }
    }

    return new Animation(tracks, action->channels.count, action->length);
}

Animation* AnimationUtils::BoneAnimationFromAction(Action* action, Armature* armature, Transform targets[4])
{
    ITrack** tracks = new ITrack*[action->channels.count];

    for (unsigned int i = 0; i < action->channels.count; ++i)
    {
        Channel* channel = action->channels.array + i;

        unsigned int j;
        // TODO: Optimize (should do binary search)
        for (j = 0; j < armature->joints.count; ++j)
            if (armature->joints.array[j].name == channel->boneName)
                break;

        mini3d_assert((j < armature->joints.count), "Can't find transform index for bone %s and action %s on armature %s", channel->boneName, action->name, armature->name);

        switch (channel->type) 
        {
            case Channel::POSITION:
                tracks[i] = new Track<Vec3>(&targets[j].pos, (Keyframe<Vec3>*)channel->animationData.array, channel->animationData.count / POSITION_KEYFRAME_SIZE);
                break;
            case Channel::ROTATION:
                tracks[i] = new Track<Quat, true>(&targets[j].rot, (Keyframe<Quat>*)channel->animationData.array, channel->animationData.count / ROTATION_KEYFRAME_SIZE);
                break;
        }
    }

    return new Animation(tracks, action->channels.count, action->length);
}

void AnimationUtils::BoneTransformsToMatrices(float boneMatrices[4][16], Transform transforms[4], const Armature* armature)
{
    for (unsigned int i = 0; i < armature->joints.count; ++i)
    {
        Joint* joint = armature->joints.array + i;
        transforms[i].rot.RotateAxis(joint->roll);
        transforms[i].pos = Vec3(joint->offset) + transforms[i] * -Vec3(joint->offset);

        if (joint->parent)
            transforms[i] = transforms[joint->parent->index] * transforms[i];

        transforms[i].ToMatrix(boneMatrices[i]);
    }
}
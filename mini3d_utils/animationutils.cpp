
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#include "animationutils.hpp"

void mini3d_assert(bool expression, const char* text, ...) ;

using namespace mini3d::utils;

Animation* AnimationUtils::AnimationFromAction(Action* action, Transform* target)
{
    ITrack** tracks = new ITrack*[action->channelCount];

    for (unsigned int i = 0; i < action->channelCount; ++i)
    {
        Action::Channel* channel = action->channels + i;
        if (!strcmp(channel->targetName, "location"))
            tracks[i] = new Track<Vec3>(&target->pos, (Keyframe<Vec3>*)channel->animationData, channel->keyframeCount);
        else if (!strcmp(channel->targetName, "rotation_quaternion"))
            tracks[i] = new Track<Quat, true>(&target->rot, (Keyframe<Quat>*)channel->animationData, channel->keyframeCount);
    }

    return new Animation(tracks, action->channelCount, action->length);
}

Animation* AnimationUtils::BoneAnimationFromAction(Action* action, Armature* armature, Transform targets[4])
{
    ITrack** tracks = new ITrack*[action->channelCount];

    for (unsigned int i = 0; i < action->channelCount; ++i)
    {
        Action::Channel* channel = action->channels + i;

        unsigned int j;
        // TODO: Optimize (should do binary search)
        for (j = 0; j < armature->jointCount; ++j)
            if (!strcmp(armature->joints[j].name, channel->boneName))
                break;

        mini3d_assert((j < armature->jointCount), "Can't find transform index for bone %s and action %s on armature %s", channel->boneName, action->name, armature->name);

        if (!strcmp(channel->targetName, "location"))
            tracks[i] = new Track<Vec3>(&targets[j].pos, (Keyframe<Vec3>*)channel->animationData, channel->keyframeCount);
        else if (!strcmp(channel->targetName, "rotation_quaternion"))
            tracks[i] = new Track<Quat, true>(&targets[j].rot, (Keyframe<Quat>*)channel->animationData, channel->keyframeCount);
    }

    return new Animation(tracks, action->channelCount, action->length);
}

void AnimationUtils::BoneTransformsToMatrices(float boneMatrices[4][16], Transform transforms[4], const Armature* armature)
{
    for (unsigned int i = 0; i < armature->jointCount; ++i)
    {
        Joint* joint = armature->joints + i;
        transforms[i].rot.RotateAxis(joint->roll);
        transforms[i].pos = Vec3(joint->offset) + transforms[i] * -Vec3(joint->offset);

        if (joint->parentIndex != NO_BONE_PARENT)
            transforms[i] = transforms[joint->parentIndex] * transforms[i];

        transforms[i].ToMatrix(boneMatrices[i]);
    }
}


void
Pointer::store(const Domain &value, State &state) const
{
    // Go through all target memory blocks for the pointer and merge
    // them with the value being stored.
    PlaceTargetMap::const_iterator it = mTargets.begin(),
        itend = mTargets.end();

    for (; it != itend; ++it)
    {
        if (it->second->mType != Target::Block)
            continue;

        const Domain *source = state.findBlock(*it->second->mTarget);
        CANAL_ASSERT(source);

        Domain *result = source->clone();
        std::vector<Domain*> destinations = dereference(result,
                                                        it->second->mOffsets);

        // When a pointer points to a single memory target, the old
        // value can be rewritten instead of merging with it to
        // increase precision.  Pointer with a single memory target is
        // very common case in LLVM as local variables are managed
        // this way as pointers to stack.
        if (mTargets.size() == 1 && destinations.size() == 1)
        {
            (*destinations.begin())->setBottom();
            (*destinations.begin())->join(value);
        }
        else
        {
            // When the pointer references multiple locations, we
            // assume that actual program run can choose any target,
            // so we merge the stored value into all existing values.
            std::vector<Domain*>::iterator itd = destinations.begin(),
                itdend = destinations.end();

            for (; itd != itdend; ++itd)
                (*itd)->join(value);
        }

        if (state.hasGlobalBlock(*it->second->mTarget))
            state.addGlobalBlock(*it->second->mTarget, result);
        else
            state.addFunctionBlock(*it->second->mTarget, result);
    }
}

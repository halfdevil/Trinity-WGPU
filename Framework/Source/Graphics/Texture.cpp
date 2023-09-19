#include "Graphics/Texture.h"

namespace Trinity
{
    Texture::~Texture()
    {
    }

    std::type_index Texture::getType() const
    {
        return typeid(Texture);
    }
}
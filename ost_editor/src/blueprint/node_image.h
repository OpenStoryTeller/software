#ifndef NODE_IMAGE_H
#define NODE_IMAGE_H

#include "node_base.h"


class NodeImage : public NodeBase
{
public:
    NodeImage();

    void compile() override;
};

#endif // NODE_IMAGE_H

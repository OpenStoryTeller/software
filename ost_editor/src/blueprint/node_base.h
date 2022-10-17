#ifndef NODEBASE_H
#define NODEBASE_H

enum class NodeType
{
    ShowImage,
    Simple,
    Tree,
    Comment,
    Houdini
};


class NodeBase
{
public:
    NodeBase(NodeType type);

    NodeType type();

    virtual void compile() = 0;

private:
    NodeType m_type;
};

#endif // NODEBASE_H

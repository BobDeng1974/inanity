#include "Expression.hpp"
#include "SequenceNode.hpp"

Expression::Expression(ptr<Node> node) : node(node) {}

ptr<Node> Expression::GetNode() const
{
	return node;
}

void Expression::Assign(Expression expression)
{
	node = expression.node;
}

void Expression::Append(Expression expression)
{
	Assign((*this, expression));
}

BEGIN_INANITY_SHADERS

Expression operator,(Expression a, Expression b)
{
	return NEW(SequenceNode(a.GetNode(), b.GetNode()));
}

END_INANITY_SHADERS

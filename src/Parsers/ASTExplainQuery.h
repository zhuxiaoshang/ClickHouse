#pragma once

#include <Parsers/ASTQueryWithOutput.h>


namespace DB
{


/// AST, EXPLAIN or other query with meaning of explanation query instead of execution
class ASTExplainQuery : public ASTQueryWithOutput
{
public:
    enum ExplainKind
    {
        ParsedAST, /// 'EXPLAIN AST SELECT ...'
        AnalyzedSyntax, /// 'EXPLAIN SYNTAX SELECT ...'
        QueryPlan, /// 'EXPLAIN SELECT ...'
    };

    ASTExplainQuery(ExplainKind kind_, bool old_syntax_)
        : kind(kind_), old_syntax(old_syntax_)
    {
    }

    String getID(char delim) const override { return "Explain" + (delim + toString(kind, old_syntax)); }
    ExplainKind getKind() const { return kind; }
    ASTPtr clone() const override
    {
        auto res = std::make_shared<ASTExplainQuery>(*this);
        res->children.clear();
        res->children.push_back(children[0]->clone());
        cloneOutputOptions(*res);
        return res;
    }

    void setExplainedQuery(ASTPtr query_)
    {
        children.emplace_back(query_);
        query = std::move(query_);
    }

    void setSettings(ASTPtr settings_)
    {
        children.emplace_back(settings_);
        ast_settings = std::move(settings_);
    }

    const ASTPtr & getExplainedQuery() const { return query; }
    const ASTPtr & getSettings() const { return ast_settings; }

protected:
    void formatQueryImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override
    {
        settings.ostr << (settings.hilite ? hilite_keyword : "") << toString(kind, old_syntax) << (settings.hilite ? hilite_none : "") << " ";
        children.at(0)->formatImpl(settings, state, frame);
    }

private:
    ExplainKind kind;
    bool old_syntax; /// "EXPLAIN AST" -> "AST", "EXPLAIN SYNTAX" -> "ANALYZE"

    ASTPtr query;
    ASTPtr ast_settings;

    static String toString(ExplainKind kind, bool old_syntax)
    {
        switch (kind)
        {
            case ParsedAST: return old_syntax ? "AST" : "EXPLAIN AST";
            case AnalyzedSyntax: return old_syntax ? "ANALYZE" : "EXPLAIN SYNTAX";
            case QueryPlan: return "EXPLAIN";
        }

        __builtin_unreachable();
    }
};

}

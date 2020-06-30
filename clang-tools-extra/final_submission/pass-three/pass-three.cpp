#include <sstream>
#include <string>
#include<fstream>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include <set>
#include <iterator>
using namespace llvm;
using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
ifstream fp;
ifstream v;
ifstream vc;
static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
class var_label {
	public:
		string name;
		string var;


};

vector<var_label> pass_one;
vector<var_label> pass_one_var;
vector<var_label>pass_one_varc;
vector<var_label>::iterator it;
vector<var_label>::iterator pos;

vector<var_label>::iterator pos1;
vector<var_label>::iterator pos2;
ASTContext *con;
clang::SourceLocation cur_label_end;
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:

  FunctionDecl *currFunc;
  LabelStmt *currLabel;
  std::vector<string> variables;
  vector<int>::iterator ip;
  set <string, greater <string> > var;
  clang::SourceLocation l;
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}
  int ran;
  //rewriting function definition
  bool is_present(string s)
  {
	  bool k=false;
	   for(it = pass_one.begin(); it != pass_one.end(); it++ )
		  if(s==it->name)
		  {
		 	k=true;
	   		pos=it;

		  }
	   return k;


  }
   bool is_labelc_present(string s)
  {
          bool k=false;
           for(it = pass_one_varc.begin(); it != pass_one_varc.end(); it++ )
                  if(s==it->name)
                  {
                        k=true;
                        pos2=it;

                  }
           return k;


  }


   bool is_label_present(string s)
  {
          bool k=false;
           for(it = pass_one_var.begin(); it != pass_one_var.end(); it++ )
                  if(s==it->name)
                  {
                        k=true;
                        pos1=it;

                  }
           return k;


  }

  string replace_colon(string str){
      int len=str.length();
      char str2[len+2];
      bool status = true;
      for (int i=0,j=0;i<=len;i++,j++)
     {
          if(status && str[i]==':')
          {
            str2[j]='(';
            str2[++j]=')';
            status = false;
          }
          else
          {
              str2[j]=str[i];
          }
      }
      return str2;
  }

  bool VisitStmt(Stmt *s) {

//	 errs()<<con

   if (isa<LabelStmt>(s)) {

      //  errs()<<"label";
      LabelStmt *d = dyn_cast<LabelStmt>(s);
      //currLabel=d;
      clang::LangOptions lopt;

      clang::SourceLocation b(d->getBeginLoc()), _e(d->getEndLoc());
      clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lopt));


      //errs()<<currLabel->getName();
      if(!currLabel||(sm.getCharacterData(e)>sm.getCharacterData(cur_label_end)))
      {
	//TheRewriter.InsertText(b,"/*",true,true);
	//TheRewriter.InsertText(e,"*/",true,true);
	TheRewriter.RemoveText(b,sm.getCharacterData(e)-sm.getCharacterData(b));

      }

      currLabel=d;
      cur_label_end=e;
    // TheRewriter.InsertText(b,"/*",true,true);
    //  TheRewriter.InsertText(e,"*/",true,true);
      //TheRewriter.RemoveText(b,sm.getCharacterData(e)-sm.getCharacterData(b));
    }

    return true;
  }

  bool is_label_func;
  clang::SourceLocation cur_func_end;
  clang::SourceLocation cur_func_beg;

  bool VisitFunctionDecl(FunctionDecl *f) {

    currFunc=f;
    currLabel=NULL;
    int len=f->getNameInfo().getName().getAsString().length();

    bool k=is_present(f->getNameInfo().getName().getAsString());
    bool h=is_labelc_present(f->getNameInfo().getName().getAsString() );
    is_label_func=k;
    cur_func_beg=f->getBeginLoc();
    cur_func_end=f->getEndLoc();

    if(k)
    {

	    TheRewriter.RemoveText(f->getBeginLoc().getLocWithOffset(len+6),pos->var.length());
	    //errs()<<"---------------------------------\n\n\n\n";
	    TheRewriter.InsertText(f->getBeginLoc().getLocWithOffset(len+6), pos2->var, true, true);
	   // errs()<<pos->var;
    }


    return true;
  }
  bool VisitDeclRefExpr(DeclRefExpr *v) {
    int len=v->getNameInfo().getName().getAsString().length();

    bool k=is_label_present(v->getNameInfo().getName().getAsString());
    if(k)
    {


            //errs()<<"---------------------------------\n\n\n\n";
          // TheRewriter.InsertText(v->getBeginLoc().getLocWithOffset(len+1), pos1->var, true, true);
           // errs()<<pos->var;
	 //  errs()<<"dorechi";
    }
    else
    {
	    if(is_label_func&&sm.getCharacterData(v->getDecl()->getBeginLoc())<sm.getCharacterData(cur_func_beg.getLocWithOffset(pos->name.length()+pos->var.length()+7))&&sm.getCharacterData(v->getDecl()->getBeginLoc())>sm.getCharacterData(cur_func_beg))
	    {
		string vu=string(sm.getCharacterData(v->getDecl()->getBeginLoc()),sm.getCharacterData(v->getDecl()->getEndLoc())-sm.getCharacterData(v->getDecl()->getBeginLoc()));
		int k =vu.find("struct");
		int g=vu.find("[");
		int h=vu.find("*");
		if(k==-1&&g==-1&&h==-1)
		{



			TheRewriter.InsertText(v->getBeginLoc(), "*", true, true);
		}
		else if(k!=-1&&g==-1&&h==-1)
		{
			int len=v->getNameInfo().getName().getAsString().length();
			TheRewriter.RemoveText(v->getBeginLoc().getLocWithOffset(len), 1);

			TheRewriter.InsertText(v->getBeginLoc().getLocWithOffset(len), "->", true, true);

		}
	    }


    }



    return true;
  }
private:
    Rewriter &TheRewriter;
//   ASTContext *astcontext;
//
	clang::SourceManager &sm=TheRewriter.getSourceMgr();
	clang::LangOptions lopt=TheRewriter.getLangOpts();

};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}

  virtual void HandleTranslationUnit(ASTContext &Context ){
	  con=&Context;
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

  }

private:
  MyASTVisitor Visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:

  MyFrontendAction() {}
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();
       // Now emit the r ewritten buffer.
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    // llvm::errs() << "** Creating AST consumer for: " << file << "\n";
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());

    return make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
  fp.open("data_var.txt");
  v.open("vnames.txt");
  vc.open("vcnames.txt");

  var_label x;
  var_label y;
  var_label z;
  //getline(fp,x.name);
  //getline(fp,x.name);
  string s;
  string type;
  string var_list;
  while(getline(fp,s))
  {
  	//errs()<<s;
	int i=0;
	/*while(s[i]!=' ' ||s[i]!=s.end() )
	{
		i+=1;
	}*/
	i=s.find(" ");

	//errs()<<"\n"<<i<<"\n";
	type=s.substr(0,i);
	var_list=s.substr(i+1,100);
	x.name=type;
	x.var=var_list;
	pass_one.push_back(x);


  }
   while(getline(v,s))
  {
        //errs()<<s;
        int i=0;
        /*while(s[i]!=' ' ||s[i]!=s.end() )
        {
                i+=1;
        }*/
        i=s.find(" ");

        //errs()<<"\n"<<i<<"\n";
        type=s.substr(0,i);
        var_list=s.substr(i+1,100);
        y.name=type;
        y.var=var_list;
        pass_one_var.push_back(y);


  }

  while(getline(vc,s))
  {
        //errs()<<s;
        int i=0;
        /*while(s[i]!=' ' ||s[i]!=s.end() )
        {
                i+=1;
        }*/
        i=s.find(" ");

        //errs()<<"\n"<<i<<"\n";
        type=s.substr(0,i);
        var_list=s.substr(i+1,100);
        z.name=type;
        z.var=var_list;
        pass_one_varc.push_back(z);


  }








 //for(it = pass_one.begin(); it != pass_one.end(); it++ )
//	errs()<<it->var;

  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());

}

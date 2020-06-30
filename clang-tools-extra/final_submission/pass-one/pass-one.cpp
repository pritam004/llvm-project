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

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
ofstream fp;
ofstream vnames;
ofstream vcnames;
clang::SourceLocation cur_label_end;
clang::SourceLocation cur_label_beg;
//fp.open("data_var.txt");
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:

  FunctionDecl *currFunc;
  LabelStmt *currLabel;
  std::vector<string> variables;
  vector<string>::iterator it;
  set <string, greater <string> > var;
  clang::SourceLocation l;
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}
  int ran;
 // ofstream fp;
 // fp.open("data_var.txt","w");
  //rewriting function definition
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


   bool is_present(string s)
  {
	  bool k=false;
	   for(it = variables.begin(); it != variables.end(); it++ )
		  if(s==*it)
		  {
		 	k=true;
	   		//pos=it;

		  }
	   return k;


  }

  int label_var_count=0;
  bool VisitStmt(Stmt *s) {

   if (isa<LabelStmt>(s)) {
	   variables.clear();
	//fp.open("data_var.txt");
        errs()<<"label";
	//fp<<"\n"<<<LabelStmt>(s)->getName()<<" ";
      LabelStmt *d = dyn_cast<LabelStmt>(s);
      currLabel=d;
	fp<<"\n"<<d->getName()<<" ";
	vnames<<"\n"<<d->getName()<<" ";
	vcnames<<"\n"<<d->getName()<<" ";
      clang::LangOptions lopt;
	label_var_count=0;
      clang::SourceLocation b(d->getBeginLoc()), _e(d->getEndLoc());
      clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lopt));

      //TheRewriter.InsertText(b,"/*",true,true);
      //TheRewriter.InsertText(e,"*/",true,true);
      ran=sm.getCharacterData(e)-sm.getCharacterData(b);
      string code = std::string(sm.getCharacterData(b), sm.getCharacterData(e)-sm.getCharacterData(b));
      TheRewriter.InsertText(currFunc->getBeginLoc(), "void ", true, true);
      l=currFunc->getBeginLoc();
            TheRewriter.InsertText(currFunc->getBeginLoc(), replace_colon(code), true, true);
      TheRewriter.InsertText(currFunc->getBeginLoc(), "\n", true, true);


      cur_label_end=e;
      cur_label_beg=b;
    }

    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *f) {
    //fp.open("data_var.txt");

    currFunc=f;
    currLabel=NULL;
    errs()<<"functiondecl";
    var_count=0;
    variables.clear();
    return true;

  }
bool VisitCXXRecordDecl (CXXRecordDecl *d)
{
	if(currFunc)
	{

		clang::SourceLocation b(d->getBeginLoc()), _e(d->getEndLoc());
      		clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lopt));

		string code = std::string(sm.getCharacterData(b), sm.getCharacterData(e)-sm.getCharacterData(b)+1);
      		TheRewriter.InsertText(currFunc->getBeginLoc(), code, true, true);
		TheRewriter.InsertText(currFunc->getBeginLoc(), "\n", true, true);

	}

	//TheRewriter.InsertText(d->getBeginLoc(), "olala", true, true);

	return true;
}


  int var_count=0;
  bool VisitDeclRefExpr(DeclRefExpr *v) {




	if(currLabel && (sm.getCharacterData(v->getBeginLoc())<sm.getCharacterData(cur_label_end))&&sm.getCharacterData(v->getDecl()->getBeginLoc())<sm.getCharacterData(cur_label_beg)&& (sm.getCharacterData(v->getDecl()->getBeginLoc())>sm.getCharacterData(currFunc->getBeginLoc())) )
	{


		//fp<<"\n"<<currLabel->getName()<<" ";

		string vu=string(sm.getCharacterData(v->getDecl()->getBeginLoc()),sm.getCharacterData(v->getDecl()->getEndLoc())-sm.getCharacterData(v->getDecl()->getBeginLoc()));
		 //errs()<<"before"<<vu<<"\n\n";

		/*while(vu.back()=='='||vu.back()==' ')
		{
			vu.pop_back();
		}*/
		int k,point,point1;
		if(vu.find("struct")==-1)
		{
			 k=vu.find(" ");
			//int point,point1;
			point=vu.find("*");
			point1=vu.find("[");
		}
		else
		{
			k=vu.find(" ");
			string vc=vu.substr(0,k);
			k=vc.find(" ");
			 point=vu.find("*");
                        point1=vu.find("[");

		}

		errs()<<"\nprinting here"<< point<<"   "<<point1;
	       	string vo=vu.substr(0,k);

		//errs()<<"after"<<vu<<"\n\n";
		if(!is_present(v->getNameInfo().getName().getAsString()))
		{

			if(label_var_count==0)
			{
				if((point==-1)&&(point1==-1))
				{
					errs()<<"\ninside first here"<< point<<"   "<<point1;
					fp<<vo<<" ";
					fp<<v->getNameInfo().getName().getAsString();
					vcnames<<vo<<" "<<"*";
                                        vcnames<<v->getNameInfo().getName().getAsString();
					vnames<<"&";
					vnames<<v->getNameInfo().getName().getAsString();
				}

				else
				{

					fp<<vo<<" ";
					fp<<"*";
                                	fp<<v->getNameInfo().getName().getAsString();
					vcnames<<vo<<" ";
                                        vcnames<<"*";
                                        vcnames<<v->getNameInfo().getName().getAsString();

                                	//vnames<<"&";
                                	vnames<<v->getNameInfo().getName().getAsString();


				}
			}
			else
			{
				if((point==-1)&&(point1==-1))
				{
					errs()<<"\ninside second here"<< point<<"   "<<point1;
					fp<<","<<vo<<" ";
					fp<<v->getNameInfo().getName().getAsString();
					vcnames<<","<<vo<<" "<<"*";
                                        vcnames<<v->getNameInfo().getName().getAsString();
					vnames<<",";
					vnames<<"&";
                        		vnames<<v->getNameInfo().getName().getAsString();
				}
				else
				{

					fp<<","<<vo<<" "<<"*";
                               		fp<<v->getNameInfo().getName().getAsString();
					vcnames<<","<<vo<<" ";
					vcnames<<"*";
                                        vcnames<<v->getNameInfo().getName().getAsString();

                               		vnames<<",";
                                	//vnames<<"&";
                                	vnames<<v->getNameInfo().getName().getAsString();
				}

			}
			label_var_count+=1;
			variables.push_back( v->getNameInfo().getName().getAsString());


		}
	}
     /*int len,c=0;

	errs()<<"\n"<<currFunc->getNameInfo().getName().getAsString ()<<"\n";
        len=currFunc->getNameInfo().getName().getAsString ().length();
        std::vector<string>::iterator it = std::find(variables.begin(), variables.end(), v->getFoundDecl()->getNameAsString ());
        if(it==variables.end())
                c=1;
        if(currLabel&&c)
	   {

			if(var_count!=0){
				 TheRewriter.InsertText(currFunc->getBeginLoc().getLocWithOffset(len+6), ",", true, true);
				TheRewriter.InsertText(currFunc->getBeginLoc().getLocWithOffset(len+6), v->getNameInfo().getName().getAsString() , true, true);
				// TheRewriter.InsertText(currFunc->getBeginLoc().getLocWithOffset(len+6),v->getDecl()->getType().getBaseTypeIdentifier ()->getNameStart() );
			//	errs()<<dynamic_cast<const VarDecl*>(v->getDecl())->getInit();
				errs()<<string(sm.getCharacterData(v->getDecl()->getBeginLoc()),3)<<"important point\n\n\n";
				variables.push_back(v->getFoundDecl()->getNameAsString ());
			 }
			else
			{

					TheRewriter.InsertText(currFunc->getBeginLoc().getLocWithOffset(len+6), v->getNameInfo().getName().getAsString() , true, true);

					 variables.push_back(v->getFoundDecl()->getNameAsString ());

			}
			errs()<<"i was here";
			var_count++;
	   }

	errs()<<len<<"\n";*/
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
       // Now emit the rewritten buffer.
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
	vnames.open("vnames.txt");
	vcnames.open("vcnames.txt");
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());

}

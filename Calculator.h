#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <list>
#include <stack>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include <ctime>


//宏定义
#define DLL_EXPORTS//导出DLL
#define INSIDE_DLL//DLL内部

#ifdef __linux__
#ifdef DLL_EXPORTS
#undef DLL_EXPORTS
#endif
#endif

//导出动态库
#ifdef DLL_EXPORTS
#ifdef INSIDE_DLL
#define TEXT_ANALYSIS_EXPORT __declspec(dllexport)
#else
#define TEXT_ANALYSIS_EXPORT __declspec(dllimport)
#endif
#else
#define TEXT_ANALYSIS_EXPORT
#endif


namespace NS_Calculator
{

	using namespace std;

	//异常类
	class Exception :public std::exception
	{
		///一些定义
	public:
		//枚举 异常类型{数值读取错误,括号不匹配,其他}
		enum ExceptionType
		{
			//Calculator类相关异常
			UnknownSymbol,//未知符号
			BracketMismatching,//括号不匹配
			NoOperand,//无操作数
			NoOperator,//无操作符
			SyntaxError,//语法错误
			MathError,//数学错误
			ExpressionNotSet,//无表达式
			ExpressionTooLong,//表达式过长
			ValueOverflow,//溢出

			//控制台异常
			UnknownCommand,//未知命令
			UnknownOption,//未知选项
			ExpectMoreOption,//期望更多选项
			ArgumentMismatching,//参数不匹配
			MutexOptionExists,//存在互斥选项
			KeyOptionNotFound,//缺少必要选项

			OtherException,//其他异常
			ExceptionType_End//哨兵
		};
		static const string exceptionDetail[ExceptionType_End];

		///常规成员
	private:
		ExceptionType exceptionType;//异常类型
		string info_extra;//额外信息

	public:
		explicit Exception(const ExceptionType& exceptionType = OtherException, string _info_extra= "No more infomations.");
		~Exception();
		virtual const char* what()const;

	};

	/**	规划一下:
	 *	1.分析字符串,得到中缀表达式
	 *	2.将中缀表达式转换为后缀表达式
	 *	3.计算后缀表达式
	 *	网址记录
	 *	https://www.cnblogs.com/hantalk/p/8734511.html
	 */


	namespace Dependencies
	{
		//检查是否是大写字符
		inline bool check_upperCaseChar(const char& c);
		inline bool check_lowerCaseChar(const char& c);
	}


	 //计算器
	class TEXT_ANALYSIS_EXPORT Calculator
	{
		//一些定义:
	public:

		struct Operator
		{
			bool enable = true;//可用性
			int priority;//优先级
			int type;//操作符的目数, 区间[0,2]
			int location;//0代表前置运算符, 1代表后置运算符或中置运算符, -1代表其它情况(括号)
			string identifier;//运算符符号
		};


		typedef int Index_operator;//操作符
		typedef int Index_constant;//常量
		typedef long double Operand;//操作数

		//枚举 操作符 {加号"+",减号"-",乘号"*",除号"/",幂"^",阶乘"!",模"%",对数"log",正弦"sin",余弦"cos",正切"tan",反正弦"arcsin",反余弦"arccos",反正切"arctan",余切"cot",排列"ARR",组合"COM",左括号"(",右括号")",正好'+',负号"-"};
		enum Operators
		{
			None = 0,//无

			//基本
			Plus,//加号"+"
			Minu,//减号"-"
			Mult,//乘号"*"
			Divi,//除号"/"
			Pow,//幂"^"

			//增强
			Fact,//阶乘"!"
			Mod,//模"%"
			Abs,//绝对值"ABS"
			Log,//对数"log"

			//三角函数
			Sin,//正弦"sin"
			Cos,//余弦"cos"
			Tan,//正切"tan"
			ASin,//反正弦"arcsin"
			ACos,//反余弦"arccos"
			ATan,//反正切"arctan"
			Cot,//余切"cot"

			//古典概型
			Arra,//排列数"ARR"
			Comb,//组合数"COM"

			//逻辑运算
			And,//与"&&"
			Or,//或"||"
			Not,//非"!!"
			Xor,//异或"!||"

			//比较运算
			JG,//大于
			JGE,//大于等于
			JL,//小于
			JLE,//小于等于
			JE,//等于
			JNE,//不等于

			//位运算
			BLS,//左移位"<<"
			BRS,//右移位">>"
			Band,//按位与"&"
			Bor,//按位或"|"
			Bnot,//按位取反"~"
			Bxor,//按位异或"!|"

			//特殊
			Dice,//骰子"D"

			BraL,//左括号"("
			BraR,//右括号")"
			Act,//正"+"
			Neg,//负"-"
			Operator_End//哨兵
		};
		//枚举 常量 {圆周率"PI",自然对数'e'}
		enum Constants
		{
			NotConstant = -1,
			PI,//圆周率"PI"
			NaLo,//自然对数"e"
			True,//真
			False,//假
			Ans,//上一次结果
			Constant_End//哨兵
		};
		//枚举 格式模式 {位数,科学,常规}
		enum FormatMode { Fix = 0, Sci, Norm };
		//枚举 角度单位 {角度,弧度}
		enum AngleUnit { Deg = 0, Rad };


		static const Operator operators[Operator_End];
		//下面某些被舍弃了! 现在封装在上面这个数组里面
		//static const int offset;//偏移量
		//static const char* operatorsIdentifier[Operator_End];//运算符标识符
		//static const int operatorsIdentifierLength[Operator_End];//运算符标识符长度
		static const Operand constantsValue[Constant_End];//常量值
		//static const int priority[Operator_End];//优先级
		//static const int operatorsType[Operator_End];//操作符目数
		//static const int operatorsPosition[Operator_End];//操作符位置
		static const Operand angleUnitsConvertConstant;//角单位转换常量
		static const size_t lenLimit_expression;//表达式长度限制



	public:


		class Node//节点
		{
		public:
			Index_operator _operator;//操作符
			Operand _operand;//操作数
		};
		typedef std::list<Node> Expression;//表达式
		typedef std::stack<Node> OperatorStack;//操作符栈
	//    typedef std::list<Node>::iterator IteratorToNode;//迭代器

		typedef std::stack<Operand> OperandStack;//操作数栈


	private:
		//常规成员
	//		std::string expression;//字符串表达式
		char* expression;//字符串表达式
		Operand result;//结果
		Expression infixExpression;//中缀表达式
		Expression postfixExpression;//后缀表达式

		FormatMode formatMode;//当前格式模式
		AngleUnit angleUnit;//角度单位
		int precision;//精度
		bool displayDigitSeparator;//显示位分隔符


	public:
		//常规成员函数

		//构造函数
		Calculator();
		Calculator(const Calculator& another) = delete;//不支持复制构造
		//析构函数
		~Calculator();

		//基本操作函数
		void setFormat(const FormatMode& format = Norm, const int& precision = 2);
		void setPrecision(const int& precision);//设置精度
		void setAngleUnit(const AngleUnit& unit = Rad);//设置角度单位
		void setDisplayDigitSeparator(const bool& displayState = false);//设置是否显示数字分隔符","

		Operand getResult()const;//获取运算结果(返回double)
		string getFormattedResult()const;//获取格式化结果(返回字符串)
		Operand calculate(const std::string& expression);//计算(便携函数, 内部调用其他函数实现)
		Operand calculate();//计算
		bool setExpression(const string& tarExpression);//设置表达式
		string getInfixExpression();//获取中缀表达式
		string getPostfixExpression();//获取后缀表达式

		Calculator& operator=(const Calculator& another) = delete;//不支持赋值操作

	//    static Operand Calculate(const std::string &expression);//静态函数计算

	private:
		//内部函数
		void _jumpToNext(int& index);
		bool _parseString();//分析字符串(可能抛出异常)
		bool _convertToPostfix();//转换为后缀表达式(可能抛出异常)
		Operand _calculatePostfix();//计算后缀表达式结果(可能抛出异常)
		Operand _getValue(const Operand& operand1, const Operand& operand2, const Index_operator& op);//计算值
		Index_operator _scanOperator(int& index);//扫描操作符,步进index(该函数总是进行最长匹配)
		Index_constant _scanConstant(int& index);//扫描常量,步进index
		int _operatorCopmare(const char* str, const Index_operator& op)noexcept;//操作符匹配(返回与指定操作符的最大匹配长度)(大小写不敏感)

	};
}









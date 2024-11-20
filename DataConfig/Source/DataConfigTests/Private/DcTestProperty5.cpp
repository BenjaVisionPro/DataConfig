#include "DcTestProperty5.h"

#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"

DC_TEST("DataConfig.Core.Property.Defaults")
{
	FString Str = TEXT(R"(
		{
			"StrFieldWithDefault" : "Bar",
			"IntFieldWithDefault" : 123,

			"StrArrFieldWithDefault" : ["One", "Two", "Three", "Four"],
			"StrSetFieldWithDefault" : ["Foo", "Bar"],
			"StringIntMapFieldWithDefault" : {
				"Five" : 5,
				"Ten" : 10
			}
		}
	)");

	FDcJsonReader Reader(Str);
	FDcTestStructWithDefaults Dest;

	UTEST_OK("Deserialize With Defaults", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Dest)));

	FDcTestStructWithDefaults Expect;
	Expect.StrFieldWithDefault = "Bar";
	Expect.IntFieldWithDefault = 123;

	Expect.StrArrFieldWithDefault = {"One", "Two", "Three", "Four"};
	Expect.StrSetFieldWithDefault = {"Foo", "Bar"};
	Expect.StringIntMapFieldWithDefault = {
		{"Five", 5},
		{"Ten", 10},
	};

	UTEST_OK("Deserialize With Defaults", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Dest), FDcPropertyDatum(&Expect)));

	return true;
}

# RefData

Responses in `//blp/refdata` use a different dynamic schema for each response,
depending on the fields requested.  This makes it is necessary to create an
ad-hoc schema for the test cases.

The example in this repository provides an example schema that supports
successful and unsuccessful `ReferenceDataResponse` types. The provided schema
can be modified to handle additional fields as needed.

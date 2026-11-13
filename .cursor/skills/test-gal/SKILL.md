---
name: test-gal
description: >-
  Mark automated integration-test incoming .gal files with // good or // bad
  on the line the case is about. examples/uuid_protocol is a test for a
  human to run by hand and must not get those comments. Use when editing
  .gal files under galimatron/tests/test_data, examples/uuid_protocol,
  validate fixtures, or wizard tests.
---

# Test .gal files

`examples/uuid_protocol` is a test for a human to run by hand. Do not add `// good` or `// bad` comments there.

Automated integration fixtures (`validate`, `wizard_create_aggregate`, `wizard_create_root_aggregate`) comment the incoming line the case is about:

```
0 = deleted;// good: deleted prefix
3;// bad: live after trailing deleted
```

A trailing comment is printed on the next declaration. Expected output for a passing case follows that shift. The comment on the last declaration is not printed.

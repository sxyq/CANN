# Attr

## 功能说明

注册算子属性参数。

当需要设置的参数不参与kernel侧计算时，可以将该参数注册为算子属性参数。

## 函数原型

```
OpAttrDef &Attr(const char *name)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| name | 输入 | 算子属性名称。 |

## 返回值说明

算子属性定义，OpAttrDef请参考[OpAttrDef](../OpAttrDef/OpAttrDef_functions.md)。

## 约束说明

Attr属性名不能与以下python关键字及内置变量名相同，否则会导致未定义错误。

-   常见python关键字参考

    and、 as、 assert、 break、 class、 continue、 def、 del、 elif、 else、 except、 finally、 for、 from、 global、 if、 import、 in、 is、 lambda、 not、 or、 pass、 raise、 return、 try、 while、 with、 yield、 False、 None、 True、 nonlocal、 arg。

-   内置变量名

    \_\_inputs\_\_、 \_\_outputs\_\_、 \_\_attrs\_\_、 options、 bisheng、 bisheng\_path、 tikcpp\_path、 impl\_mode、 custom\_compile\_options、 custom\_all\_compile\_options、 soc\_version、 soc\_short、 custom\_compile\_options\_soc、 custom\_all\_compile\_options\_soc、 origin\_func\_name、 ascendc\_src\_dir\_ex、 ascendc\_src\_dir、 ascendc\_src\_file、 src、 op\_type、 code\_channel、 op\_info、 compile\_op、 get\_code\_channel、 result、 isinstance、 attr、 get\_current\_build\_config、 \_build\_args、 get\_dtype\_fmt\_options、 shutil、 os、 get\_kernel\_source、ascendc\_api\_version\_header\_path、ascendc\_api\_version\_file、ascendc\_api\_version、re。

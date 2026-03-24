import com.riversideresearch.hammer.*;

/**
 * Basic smoke tests for the Hammer Java bindings, mirroring hammer_tests.py.
 *
 * The JNI library must be loadable via java.library.path. The SConscript sets
 * this up automatically when running through the build system.
 */
public class HammerTests {

    static {
        System.loadLibrary("hammer_jni");
    }

    // Token type constants — mirror of HTokenType_ in hammer.h.
    static final int TT_NONE     = 1;
    static final int TT_BYTES    = 2;
    static final int TT_SINT     = 4;
    static final int TT_UINT     = 8;
    static final int TT_SEQUENCE = 16;

    static int passed = 0;
    static int failed = 0;

    static void assertTrue(String name, boolean cond) {
        if (cond) {
            passed++;
        } else {
            failed++;
            System.err.println("FAIL: " + name);
        }
    }

    static void assertNull(String name, Object obj) {
        assertTrue(name + " (expected null)", obj == null);
    }

    static void assertNotNull(String name, Object obj) {
        assertTrue(name + " (expected non-null)", obj != null);
    }

    static void assertEqual(String name, long expected, long actual) {
        if (expected != actual) {
            failed++;
            System.err.println("FAIL: " + name + " — expected " + expected + ", got " + actual);
        } else {
            passed++;
        }
    }

    // -------------------------------------------------------------------------

    static void testToken() {
        byte[] input = {(byte)0x39, (byte)0x35, (byte)0xa2};
        HParser p = hammer.h_token(input);

        HParseResult r = p.parse(input);
        assertNotNull("token:success", r);
        assertTrue("token:type",   r.getAst().tokenType() == TT_BYTES);
        assertEqual("token:length", 3, r.getAst().bytesLength());
        assertEqual("token:byte0",  0x39, r.getAst().byteAt(0));
        assertEqual("token:byte1",  0x35, r.getAst().byteAt(1));
        assertEqual("token:byte2",  0xa2, r.getAst().byteAt(2) & 0xff);

        assertNull("token:partial_fail", p.parse(new byte[]{(byte)0x39, (byte)0x35}));
    }

    static void testCh() {
        HParser p = hammer.h_ch((short)0xa2);

        HParseResult r = p.parse(new byte[]{(byte)0xa2});
        assertNotNull("ch:success", r);
        assertTrue("ch:type",  r.getAst().tokenType() == TT_UINT);
        assertEqual("ch:value", 0xa2L, r.getAst().uintValue());

        assertNull("ch:fail", p.parse(new byte[]{(byte)0xa3}));
    }

    static void testChRange() {
        HParser p = hammer.h_ch_range((short)'a', (short)'c');

        assertNotNull("ch_range:success",  p.parse(new byte[]{(byte)'b'}));
        assertNull("ch_range:fail",        p.parse(new byte[]{(byte)'d'}));
    }

    static void testInt64() {
        HParser p = hammer.h_int64();
        byte[] input = {(byte)0xff,(byte)0xff,(byte)0xff,(byte)0xfe,
                        (byte)0x00,(byte)0x00,(byte)0x00,(byte)0x00};

        HParseResult r = p.parse(input);
        assertNotNull("int64:success", r);
        assertTrue("int64:type",   r.getAst().tokenType() == TT_SINT);
        assertEqual("int64:value", -0x200000000L, r.getAst().sintValue());

        assertNull("int64:fail", p.parse(new byte[]{
            (byte)0xff,(byte)0xff,(byte)0xff,(byte)0xfe,
            (byte)0x00,(byte)0x00,(byte)0x00}));
    }

    static void testInt32() {
        HParser p = hammer.h_int32();

        HParseResult r = p.parse(new byte[]{(byte)0xff,(byte)0xfe,(byte)0x00,(byte)0x00});
        assertNotNull("int32:success", r);
        assertEqual("int32:value", -0x20000L, r.getAst().sintValue());

        assertNull("int32:fail", p.parse(new byte[]{(byte)0xff,(byte)0xfe,(byte)0x00}));
    }

    static void testUint64() {
        HParser p = hammer.h_uint64();
        byte[] input = {(byte)0x00,(byte)0x00,(byte)0x00,(byte)0x02,
                        (byte)0x00,(byte)0x00,(byte)0x00,(byte)0x00};

        HParseResult r = p.parse(input);
        assertNotNull("uint64:success", r);
        assertTrue("uint64:type",   r.getAst().tokenType() == TT_UINT);
        assertEqual("uint64:value", 0x200000000L, r.getAst().uintValue());

        assertNull("uint64:fail", p.parse(new byte[]{
            (byte)0x00,(byte)0x00,(byte)0x00,(byte)0x02,
            (byte)0x00,(byte)0x00,(byte)0x00}));
    }

    static void testUint32() {
        HParser p = hammer.h_uint32();

        HParseResult r = p.parse(new byte[]{(byte)0x00,(byte)0x02,(byte)0x00,(byte)0x00});
        assertNotNull("uint32:success", r);
        assertEqual("uint32:value", 0x20000L, r.getAst().uintValue());

        assertNull("uint32:fail", p.parse(new byte[]{(byte)0x00,(byte)0x02,(byte)0x00}));
    }

    static void testUint8() {
        HParser p = hammer.h_uint8();

        HParseResult r = p.parse(new byte[]{(byte)0x78});
        assertNotNull("uint8:success", r);
        assertEqual("uint8:value", 0x78L, r.getAst().uintValue());

        assertNull("uint8:fail", p.parse(new byte[]{}));
    }

    static void testIntRange() {
        HParser p = hammer.h_int_range(hammer.h_uint8(), 3, 10);

        HParseResult r = p.parse(new byte[]{5});
        assertNotNull("int_range:success", r);
        assertEqual("int_range:value", 5L, r.getAst().uintValue());

        assertNull("int_range:fail", p.parse(new byte[]{11}));
    }

    static void testSequence() {
        HParser p = hammer.h_sequence__a(new HParser[]{
            hammer.h_ch((short)'a'),
            hammer.h_ch((short)'b'),
        });

        HParseResult r = p.parse(new byte[]{(byte)'a', (byte)'b'});
        assertNotNull("sequence:success", r);
        assertTrue("sequence:type",   r.getAst().tokenType() == TT_SEQUENCE);
        assertEqual("sequence:length", 2L, r.getAst().seqLength());
        assertEqual("sequence:elem0", 'a', r.getAst().seqElement(0).uintValue());
        assertEqual("sequence:elem1", 'b', r.getAst().seqElement(1).uintValue());

        assertNull("sequence:fail_partial", p.parse(new byte[]{(byte)'a'}));
        assertNull("sequence:fail_wrong",   p.parse(new byte[]{(byte)'b'}));
    }

    static void testChoice() {
        HParser p = hammer.h_choice__a(new HParser[]{
            hammer.h_ch((short)'a'),
            hammer.h_ch((short)'b'),
        });

        HParseResult r1 = p.parse(new byte[]{(byte)'a'});
        assertNotNull("choice:a_success", r1);
        assertEqual("choice:a_value", 'a', r1.getAst().uintValue());

        HParseResult r2 = p.parse(new byte[]{(byte)'b'});
        assertNotNull("choice:b_success", r2);
        assertEqual("choice:b_value", 'b', r2.getAst().uintValue());

        assertNull("choice:fail", p.parse(new byte[]{(byte)'c'}));
    }

    static void testMany() {
        HParser p = hammer.h_many(hammer.h_ch((short)'a'));

        HParseResult r = p.parse(new byte[]{(byte)'a',(byte)'a',(byte)'a'});
        assertNotNull("many:success", r);
        assertEqual("many:count", 3L, r.getAst().seqLength());

        HParseResult r0 = p.parse(new byte[]{});
        assertNotNull("many:empty", r0);
        assertEqual("many:empty_count", 0L, r0.getAst().seqLength());
    }

    static void testMany1() {
        HParser p = hammer.h_many1(hammer.h_ch((short)'a'));

        HParseResult r = p.parse(new byte[]{(byte)'a',(byte)'a'});
        assertNotNull("many1:success", r);
        assertEqual("many1:count", 2L, r.getAst().seqLength());

        assertNull("many1:fail_empty", p.parse(new byte[]{}));
    }

    static void testEndP() {
        HParser p = hammer.h_sequence__a(new HParser[]{
            hammer.h_ch((short)'a'),
            hammer.h_end_p(),
        });

        assertNotNull("end_p:success",     p.parse(new byte[]{(byte)'a'}));
        assertNull("end_p:fail_trailing",  p.parse(new byte[]{(byte)'a', (byte)'a'}));
    }

    static void testOptional() {
        HParser p = hammer.h_sequence__a(new HParser[]{
            hammer.h_ch((short)'a'),
            hammer.h_optional(hammer.h_ch((short)'b')),
            hammer.h_ch((short)'c'),
        });

        HParseResult r1 = p.parse(new byte[]{(byte)'a',(byte)'b',(byte)'c'});
        assertNotNull("optional:abc_success", r1);
        assertEqual("optional:abc_length", 3L, r1.getAst().seqLength());

        HParseResult r2 = p.parse(new byte[]{(byte)'a',(byte)'c'});
        assertNotNull("optional:ac_success", r2);
        assertEqual("optional:ac_length", 3L, r2.getAst().seqLength());
        assertTrue("optional:ac_middle_none",
            r2.getAst().seqElement(1).tokenType() == TT_NONE);

        assertNull("optional:fail", p.parse(new byte[]{(byte)'a',(byte)'e',(byte)'c'}));
    }

    static void testSepBy() {
        HParser p = hammer.h_sepBy(
            hammer.h_choice__a(new HParser[]{
                hammer.h_ch((short)'1'),
                hammer.h_ch((short)'2'),
                hammer.h_ch((short)'3'),
            }),
            hammer.h_ch((short)',')
        );

        HParseResult r = p.parse(new byte[]{(byte)'1',(byte)',',(byte)'2',(byte)',',(byte)'3'});
        assertNotNull("sepBy:success", r);
        assertEqual("sepBy:count", 3L, r.getAst().seqLength());

        HParseResult r0 = p.parse(new byte[]{});
        assertNotNull("sepBy:empty", r0);
        assertEqual("sepBy:empty_count", 0L, r0.getAst().seqLength());
    }

    // -------------------------------------------------------------------------

    public static void main(String[] args) {
        testToken();
        testCh();
        testChRange();
        testInt64();
        testInt32();
        testUint64();
        testUint32();
        testUint8();
        testIntRange();
        testSequence();
        testChoice();
        testMany();
        testMany1();
        testEndP();
        testOptional();
        testSepBy();

        System.out.printf("Results: %d passed, %d failed%n", passed, failed);
        if (failed > 0) System.exit(1);
    }
}

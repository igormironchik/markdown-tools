export const html = `<!DOCTYPE html>
<article>
    <blockquote><p> [!IMPORTANT] 
    text </p></blockquote>

    <blockquote><p> [!NOTE] text </p></blockquote>

    <blockquote><p> [!TIP] text </p></blockquote>

    <blockquote><p> [!WARNING] text </p></blockquote>

    <blockquote><p> [!CAUTION] text </p></blockquote>

    <blockquote class="modified"><p> [!NOTE]
     [!TIP] </p></blockquote>

    <blockquote class="not-modified"><p> [!TIP] [!NOTE] </p></blockquote>

    <ul>
        <li><blockquote class="not-modified-nested"><p> [!TIP] </p></blockquote></li>
    </ul>

    <blockquote class="not-modified-p-is-not-first"><ul><li>text</li></ul><p> [!TIP] </p></blockquote>
</article>
`;
